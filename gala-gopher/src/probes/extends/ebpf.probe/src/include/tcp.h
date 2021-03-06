/******************************************************************************
 * Copyright (c) Huawei Technologies Co., Ltd. 2021. All rights reserved.
 * gala-gopher licensed under the Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *     http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY OR FIT FOR A PARTICULAR
 * PURPOSE.
 * See the Mulan PSL v2 for more details.
 * Author: algorithmofdish
 * Create: 2021-09-28
 * Description: provide gala-gopher epbf endpoint functions
 ******************************************************************************/
#ifndef __TCP_H__
#define __TCP_H__

#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#ifdef COMM_LEN
#undef COMM_LEN
#endif
#define COMM_LEN 64

struct tcp_listen_port {
    unsigned int pid;
    unsigned int port;
    char comm[COMM_LEN];
};

#define LTP_MAX_NUM 1024
#define PORT_MAX_NUM 65536
struct tcp_listen_ports {
    unsigned int tlp_num;
    struct tcp_listen_port *tlp[LTP_MAX_NUM];
    unsigned int tlp_hash[PORT_MAX_NUM];
};

#ifdef IP_LEN
#undef IP_LEN
#endif
#define IP_LEN 128

struct ip_addr {
    char ip[IP_LEN];
    unsigned int port;
    int ipv4;
};

struct tcp_estab_comm {
    char comm[COMM_LEN];
    unsigned int pid;
    unsigned int fd;
};
#define TCP_ESTAB_COMM_MAX 32
struct tcp_estab {
    int is_client;
    int te_comm_num;
    struct ip_addr local;
    struct ip_addr remote;
    struct tcp_estab_comm *te_comm[TCP_ESTAB_COMM_MAX];
};


#define TCP_ESTAB_MAX 1024
struct tcp_estabs {
    unsigned int te_num;
    struct tcp_estab *te[TCP_ESTAB_MAX];
};

enum endpoint_type{
    EP_TYPE_LISTEN = 1,
    EP_TYPE_IP,
    EP_TYPE_IP_PORT
};

enum endpoint_addr_type{
    EP_ADDR_IPV4 = 1,
    EP_ADDR_IPV6
};

struct tcp_endpoint {
    enum endpoint_type ep_type;
    enum endpoint_addr_type ep_addr_type;
    unsigned int port;
    unsigned int pid;
    
    union {
        struct in6_addr in6_addr;
        struct in_addr in_addr;
    } addr; // network byte order
};

#define TCP_ENDPOINT_MAX 1024
struct tcp_endpoints {
    unsigned int tep_num;
    struct tcp_endpoint *tep[TCP_ENDPOINT_MAX];
};

char is_listen_port(unsigned int port, struct tcp_listen_ports* tlps);
struct tcp_listen_ports* get_listen_ports();
void free_listen_ports(struct tcp_listen_ports** ptlps);
struct tcp_estabs* get_estab_tcps(struct tcp_listen_ports* tlps);
void free_estab_tcps(struct tcp_estabs** ptes);
struct tcp_endpoints *get_tcp_endpoints(struct tcp_listen_ports* tlps, struct tcp_estabs* tes);
void free_tcp_endpoints(struct tcp_endpoints **pteps);


#endif
