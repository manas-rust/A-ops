#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/resource.h>

#ifdef BPF_PROG_KERN
#undef BPF_PROG_KERN
#endif

#ifdef BPF_PROG_USER
#undef BPF_PROG_USER
#endif

#include "bpf.h"
#include "tcpprobe.skel.h"
#include "tcpprobe.h"
#include "tcp.h"
#include "args.h"

#define OO_NAME "tcp_link"  // Observation Object name

#define FILTER_LISTEN_PORT 22
#define FILTER_COMM "sshd"
#define FILTER_COMM2 "broker"

static volatile sig_atomic_t stop;
static struct probe_params params = {.period = 5};

static int __is_filter_listen_port(int port) {
    if (port == FILTER_LISTEN_PORT) {
        return 1;
    }
    return 0;
}

static int __is_filter_comm(char *comm) {
    if (strstr(comm, FILTER_COMM)) {
        return 1;
    }
    if (strstr(comm, FILTER_COMM2)) {
        return 1;
    }
    return 0;
}


static void sig_int(int signo)
{
    stop = 1;
}

void update_link_metric_data(struct metric_data *dd, struct link_data *d)
{
    __u32 tmp_srtt_max = dd->srtt_max;

    if (dd->link_num == 0) {
        memcpy(dd->comm, d->comm, TASK_COMM_LEN);
    }

    dd->link_num++;
    dd->rx += d->rx;
    dd->tx += d->tx;

    /*
        metrics_rtt * (metrics_segs_in + metrics_segs_out) + link_rtt * (link_segs_in + link_segs_out)
       _________________________________________________________________________________________________
       
       (metrics_segs_in + metrics_segs_out + link_segs_in + link_segs_out)
    */ 
    if ((d->segs_in + d->segs_out + dd->segs_in + dd->segs_out)) {
        dd->srtt = (dd->srtt * (dd->segs_in + dd->segs_out) + d->srtt * (d->segs_in + d->segs_out)) /
                   (d->segs_in + d->segs_out + dd->segs_in + dd->segs_out);
    } else {
        dd->srtt = 0;
    }

    dd->srtt_max = tmp_srtt_max > d->srtt ? tmp_srtt_max : d->srtt;

    if (0 == dd->rcv_wnd_min) {
        dd->rcv_wnd_min = d->rcv_wnd;
    } else {
        if (d->rcv_wnd < dd->rcv_wnd_min) {
            dd->rcv_wnd_min = d->rcv_wnd;
        }
    }

    if (0 == dd->rcv_wnd_max) {
        dd->rcv_wnd_max = d->rcv_wnd;
    } else {
        if (d->rcv_wnd > dd->rcv_wnd_max) {
            dd->rcv_wnd_max = d->rcv_wnd;
        }
    }

    if (0 == dd->rcv_wnd_avg || dd->link_num == 1) {
        dd->rcv_wnd_avg = d->rcv_wnd;
    } else {
        dd->rcv_wnd_avg = (dd->rcv_wnd_avg * (dd->link_num - 1) + d->rcv_wnd) / dd->link_num;
    }
    
    dd->segs_in += d->segs_in;
    dd->segs_out += d->segs_out;
    dd->total_retrans += d->total_retrans;
    dd->lost += d->lost_out;
    dd->backlog_drops += d->backlog_drops;
    dd->sk_drops += d->sk_drops;
    dd->md5_hash_drops += d->md5_hash_drops;
    dd->filter_drops += d->filter_drops;
    dd->ofo_count += d->ofo_count;
}

void update_link_metric_map(struct link_key *k, struct link_data *d, int map_fd)
{
    struct metric_key key = {0};
    struct metric_data data = {0};

    /* Filtering redundant data */
    if (__is_filter_comm(d->comm)) {
        return;
    }

    /* build key */
    if (d->role == LINK_ROLE_CLIENT) {
        memcpy((char *)&key.c_ip, (char *)&k->src_addr, sizeof(struct ip));
        memcpy((char *)&key.s_ip, (char *)&k->dst_addr, sizeof(struct ip));
        key.s_port = ntohs(k->dst_port);
    } else {
        memcpy((char *)&key.s_ip, (char *)&k->src_addr, sizeof(struct ip));
        memcpy((char *)&key.c_ip, (char *)&k->dst_addr, sizeof(struct ip));
        key.s_port = k->src_port;
    }
    key.proto = k->family;
    key.pid = d->pid;
    data.role = d->role;

    bpf_map_lookup_elem(map_fd, &key, &data);
    update_link_metric_data(&data, d);

    bpf_map_update_elem(map_fd, &key, &data, BPF_ANY);
    return;
}

void pull_probe_data(int map_fd, int metric_map_fd)
{
    int ret;
    struct link_key key = {0};
    struct link_key next_key = {0};
    struct link_data data;
    unsigned char src_ip_str[INET6_ADDRSTRLEN];
    unsigned char dst_ip_str[INET6_ADDRSTRLEN];

    while (bpf_map_get_next_key(map_fd, &key, &next_key) != -1) {
        ret = bpf_map_lookup_elem(map_fd, &next_key, &data);
        if (ret == 0) {

            ip_str(next_key.family, (unsigned char *)&(next_key.src_addr), src_ip_str, INET6_ADDRSTRLEN);
            ip_str(next_key.family, (unsigned char *)&(next_key.dst_addr), dst_ip_str, INET6_ADDRSTRLEN);
            /*
            printf("===[%s:%u]: src_addr:%s:%u, dst_addr:%s:%u, family:%u, role:%s, states:%x, rx:%llu, tx:%llu, "
                   "seg_in:%u, segs_out:%u, total_retrans:%u, lost:%u, srtt:%uus, sk_err:%d, sk_err_soft:%d\n",
                data.comm,
                data.pid,
                src_ip_str,
                next_key.src_port,
                dst_ip_str,
                next_key.dst_port,
                next_key.family,
                (data.role ? "client" : "server"),
                data.states,
                data.rx,
                data.tx,
                data.segs_in,
                data.segs_out,
                data.total_retrans,
                data.lost,
                data.srtt,
                data.sk_err,
                data.sk_err_soft);
            */
            /* ??????link metric */
            update_link_metric_map(&next_key, &data, metric_map_fd);
        }

        if (data.states & (1 << TCP_CLOSE)) {
            bpf_map_delete_elem(map_fd, &next_key);
        } else {
            key = next_key;
        }
    }
    //printf("=========\n\n\n\n");
    return;
}

void print_link_metric(int map_fd)
{
    int ret = 0;
    struct metric_key key = {0}, next_key = {0};
    struct metric_data data = {0};

    unsigned char src_ip_str[INET6_ADDRSTRLEN];
    unsigned char dst_ip_str[INET6_ADDRSTRLEN];

    char *tm = get_cur_time();
    while (bpf_map_get_next_key(map_fd, &key, &next_key) != -1) {
        ret = bpf_map_lookup_elem(map_fd, &next_key, &data);
        if (ret == 0) {
            ip_str(next_key.proto, (unsigned char *)&(next_key.c_ip), src_ip_str, INET6_ADDRSTRLEN);
            ip_str(next_key.proto, (unsigned char *)&(next_key.s_ip), dst_ip_str, INET6_ADDRSTRLEN);
            fprintf(stdout,
                "|%s|%u|%s|%d|%s|%s|%u|%u|%u|%llu|%llu|%u|%u|%u|%u|%u|%u|%u|%u|%u|%u|%u|%u|%u|%u|\n",
                OO_NAME,
                next_key.pid,
                data.comm,
                data.role,
                src_ip_str,
                dst_ip_str,
                next_key.s_port,
                next_key.proto,
                data.link_num,
                data.rx,
                data.tx,
                data.segs_in,
                data.segs_out,
                data.total_retrans,
                data.lost,
                data.srtt,
                data.srtt_max,
                data.rcv_wnd_min,
                data.rcv_wnd_avg,
                data.rcv_wnd_max,
                data.backlog_drops,
                data.sk_drops,
                data.md5_hash_drops,
                data.filter_drops,
                data.ofo_count);

            printf("%s [%u-%s]: c_ip:%s, s_ip:%s:%u, proto:%u, link_num:%u, rx:%llu, tx:%llu, "
                   "segs_in:%u, segs_out:%u, total_retrans:%u, lost:%u, srtt:%uus, srtt_max:%uus, "
                   "rcv_wnd_min:%u, rcv_wnd_avg:%u, rcv_wnd_max:%u, backlog:%u, sk_drop:%u, "
                   "md5:%u, filter:%u, ofo:%u\n",
                tm,
                next_key.pid,
                data.comm,
                src_ip_str,
                dst_ip_str,
                next_key.s_port,
                next_key.proto,
                data.link_num,
                data.rx,
                data.tx,
                data.segs_in,
                data.segs_out,
                data.total_retrans,
                data.lost,
                data.srtt,
                data.srtt_max,
                data.rcv_wnd_min,
                data.rcv_wnd_avg,
                data.rcv_wnd_max,
                data.backlog_drops,
                data.sk_drops,
                data.md5_hash_drops,
                data.filter_drops,
                data.ofo_count);
        }

        bpf_map_delete_elem(map_fd, &next_key);
    }
    fflush(stdout);
    return;
}

void bpf_add_long_link_info_to_map(int map_fd, __u32 proc_id, int fd, __u8 role)
{
    struct long_link_info ll = {0};

    bpf_map_lookup_elem(map_fd, &proc_id, &ll);
    ll.fds[ll.cnt] = fd;
    ll.fd_role[ll.cnt] = role;
    ll.cnt++;
    bpf_map_update_elem(map_fd, &proc_id, &ll, BPF_ANY);
}

void bpf_update_long_link_info_to_map(int long_link_map_fd, int listen_port_map_fd) {
    int i, j;
    __u8 role;
    unsigned short listen_port;
    struct tcp_listen_ports* tlps = NULL;
    struct tcp_estabs* tes = NULL;

    tlps = get_listen_ports();
    if (tlps == NULL) {
        goto err;
    }
    
    /* insert listen ports into map */
    for (i = 0; i < tlps->tlp_num; i++) {
        listen_port = (unsigned short)tlps->tlp[i]->port;
        if (!__is_filter_listen_port((int)listen_port)) {
            printf("Update listen port:%u\n", listen_port);
            bpf_map_update_elem(listen_port_map_fd, &listen_port, &listen_port, BPF_ANY);
        }
    }

    tes = get_estab_tcps(tlps);
    if (tes == NULL) {
        goto err;
    }

    /* insert tcp establish into map */
    for (i = 0; i < tes->te_num; i++) {
        role = tes->te[i]->is_client == 1 ? LINK_ROLE_CLIENT : LINK_ROLE_SERVER;
        for (j = 0; j < tes->te[i]->te_comm_num; j++) {
            if (!__is_filter_comm(tes->te[i]->te_comm[j]->comm)) {
                bpf_add_long_link_info_to_map(long_link_map_fd, 
                    (__u32)tes->te[i]->te_comm[j]->pid, (int)tes->te[i]->te_comm[j]->fd, role);
                
                printf("Update establish(pid = %u, fd = %d, role = %u)\n", 
                    (__u32)tes->te[i]->te_comm[j]->pid, (int)tes->te[i]->te_comm[j]->fd, role);
            }
        }
    }

err:
    if (tlps) {
        free_listen_ports(&tlps);
    }
    if (tes) {
        free_estab_tcps(&tes);
    }
    return;
}


int main(int argc, char **argv)
{
    int err = -1;
    int metric_map_fd = -1;

    err = args_parse(argc, argv, "t:", &params);
    if (err != 0) {
        return -1;
    }
    
    printf("arg parse interval time:%us\n", params.period);

    LOAD(tcpprobe);
    
    if (signal(SIGINT, sig_int) == SIG_ERR) {
        fprintf(stderr, "can't set signal handler: %s\n", strerror(errno));
        goto err;
    }

    /* create metric hs map */
    metric_map_fd =
        bpf_create_map(BPF_MAP_TYPE_HASH, sizeof(struct metric_key), sizeof(struct metric_data), LINK_MAX_ENTRIES, 0);
    if (metric_map_fd < 0) {
        fprintf(stderr, "bpf_create_map metric map fd failed.\n");
        goto err;
    }

    /* update long link info */
    bpf_update_long_link_info_to_map(GET_MAP_FD(long_link_map), GET_MAP_FD(listen_port_map));

    printf("Successfully started!\n");

    while (!stop) {
        pull_probe_data(GET_MAP_FD(link_map), metric_map_fd);
        print_link_metric(metric_map_fd);
        sleep(params.period);
    }

err:
    if (metric_map_fd >= 0) {
        close(metric_map_fd);
    }
    UNLOAD(tcpprobe);
    return -err;
}
