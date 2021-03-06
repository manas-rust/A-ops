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
#include "taskprobe.skel.h"
#include "taskprobe.h"
#include "args.h"

#define TASK_PROBE_IO_PATH "cat /proc/%d/io"
#define TASK_PROBE_STAT_PATH "cat /proc/%d/stat"
#define TASK_PROBE_SMAPS_PATH "cat /proc/%d/smaps"

#define OO_NAME_TASK "task"

#define TASK_PROBE_COLLECTION_PERIOD 5

static volatile sig_atomic_t stop = 0;
static struct probe_params tp_params = {.period = TASK_PROBE_COLLECTION_PERIOD};

static void sig_int(int signal)
{
    stop = 1;
}

void task_probe_pull_probe_data(int map_fd)
{
    int ret;
    struct task_key ckey = {0};
    struct task_key nkey = {0};
    struct task_kdata tkd;

    while (bpf_map_get_next_key(map_fd, &ckey, &nkey) != -1) {
        ret = bpf_map_lookup_elem(map_fd, &nkey, &tkd);
        if (ret == 0) {
            fprintf(stdout, "|%s|%u|%u|%u|\n", OO_NAME_TASK, nkey.tgid, nkey.pid, tkd.fork_count);
        }
        ckey = nkey;
    }

    return;
}

int main(int argc, char **argv)
{
    int ret = -1;

    ret = signal(SIGINT, sig_int);
    if (ret < 0) 
    {
        printf("Can't set signal handler: %s\n", strerror(errno));
        goto err;
    }

    ret = args_parse(argc, argv, "t:", &tp_params);
    if (ret != 0) {
        return ret;
    }

    printf("Task probe starts with period: %us.\n", tp_params.period);

    LOAD(taskprobe);

    while (!stop) {
        task_probe_pull_probe_data(bpf_map__fd(skel->maps.task_map));
        sleep(tp_params.period);
    }

err:
    UNLOAD(taskprobe);
    return ret;
}
