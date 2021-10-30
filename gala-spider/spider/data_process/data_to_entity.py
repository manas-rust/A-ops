import os
import sys
import ast
import json
from spider.util.entityid import node_entity_name
from spider.util.entityid import edge_entity_name
from spider.util.conf import temp_tcp_file
from spider.util.conf import temp_other_file
from spider.util.conf import exclude_ip

def tcp_entity_process():
    s_nodes_table = {}
    c_edges_table = {}
    c_edges_infos = {}
    edges_table = {}
    edges_infos = {}
    if os.path.exists(temp_tcp_file):
        f = open(temp_tcp_file)
    else:
        print("/var/tmp/spider/tcpline.txt not here.")
        return None, None
    lines = f.readline()
    while lines:
        # obtain label = hostname + process_name
        line_json = json.loads(lines)
        hostname = line_json.get("hostname")
        process_name = line_json.get("process_name")
        # obtain s_port + client_ip + server_ip
        s_port = line_json.get("server_port")
        c_ip = line_json.get("client_ip")
        s_ip = line_json.get("server_ip")
        if line_json.get("client_ip") in ast.literal_eval(exclude_ip):
            continue
        if line_json.get("server_ip") in ast.literal_eval(exclude_ip):
            continue
        if line_json.get("table_name") == "ipvs_link":
            v_ip = line_json.get("virtual_ip")
            l_ip = line_json.get("local_ip")
            s_nodes_table.setdefault((v_ip, s_port), {}).setdefault('h', hostname)
            s_nodes_table.setdefault((v_ip, s_port), {}).setdefault('p', process_name)
            c_edges_table.setdefault((l_ip, s_ip, s_port), {}).setdefault('1', {}).setdefault('h', hostname)
            c_edges_table.setdefault((l_ip, s_ip, s_port), {}).setdefault('1', {}).setdefault('p', process_name)
        elif line_json.get("table_name") == "tcp_link" and c_ip != s_ip:
            role = line_json.get("role")
            if role == '0':
                s_nodes_table.setdefault((s_ip, s_port), {}).setdefault('h', hostname)
                s_nodes_table.setdefault((s_ip, s_port), {}).setdefault('p', process_name)
                c_edges_table.setdefault((c_ip, s_ip, s_port), {}).setdefault('0', {}).setdefault('h', hostname)
                c_edges_table.setdefault((c_ip, s_ip, s_port), {}).setdefault('0', {}).setdefault('p', process_name)
                c_edges_infos.setdefault((c_ip, s_ip, s_port),
                                         {"rx_bytes": line_json.get("rx_bytes", ""),
                                          "tx_bytes": line_json.get("tx_bytes", ""),
                                          "packets_out": line_json.get("packets_out", ""),
                                          "packets_in": line_json.get("packets_in", ""),
                                          "retran_packets": line_json.get("retran_packets", ""),
                                          "lost_packets": line_json.get("lost_packets", ""),
                                          "rtt": line_json.get("rtt", ""),
                                          "link_count": line_json.get("link_count", "")})
            elif role == '1':
                temp = hostname + '.' + process_name
                edges_table.setdefault((c_ip, s_ip, s_port, temp), {}).setdefault('1', {}).setdefault('h', hostname)
                edges_table.setdefault((c_ip, s_ip, s_port, temp), {}).setdefault('1', {}).setdefault('p', process_name)
                edges_infos.setdefault((c_ip, s_ip, s_port, temp),
                                       {"rx_bytes":line_json.get("rx_bytes", ""),
                                        "tx_bytes":line_json.get("tx_bytes", ""),
                                        "packets_out":line_json.get("packets_out", ""),
                                        "packets_in":line_json.get("packets_in", ""),
                                        "retran_packets":line_json.get("retran_packets", ""),
                                        "lost_packets":line_json.get("lost_packets", ""),
                                        "rtt":line_json.get("rtt", ""),
                                        "link_count":line_json.get("link_count", "")})
        lines = f.readline()

    for key in c_edges_table.keys():
        if c_edges_table[key].get('0') is not None and c_edges_table[key].get('1') is not None:
            temp = c_edges_table[key]['1']['h'] + '.' + c_edges_table[key]['1']['p']
            edges_table.setdefault((key[0], key[1], key[2], temp), c_edges_table[key])
            edges_infos.setdefault((key[0], key[1], key[2], temp), c_edges_infos[key])

    for key in edges_table.keys():
        node_key = (key[1], key[2])
        # fill edge_table '0' according to knowing nodes and lvs_edges
        if edges_table[key].get('0') is None and s_nodes_table.get(node_key) is not None:
            edges_table.setdefault(key, {}).setdefault('0', s_nodes_table[node_key])
    return edges_table, edges_infos


def lb_entity_process():
    lb_tables = {}
    if os.path.exists(temp_other_file):
        f = open(temp_other_file)
    else:
        print("/var/tmp/spider/otherline.txt not here.")
        return None
    lines = f.readline()
    while lines:
        line_json = json.loads(lines)
        hostname = line_json.get("hostname")
        table_name = line_json.get("table_name")
        if table_name == "dnsmasq_link":
            s_port = "8888"
            #lb_tables.setdefault((hostname, process_name), {}).setdefault("c-v", (c_ip, v_ip, s_port))
        else:
            c_ip = line_json.get("client_ip")
            v_ip = line_json.get("virtual_ip")
            if table_name == "ipvs_link":
                l_ip = line_json.get("local_ip")
            else:
                l_ip = v_ip
            s_ip = line_json.get("server_ip")
            s_port = line_json.get("server_port")
            v_port = line_json.get("virtual_port")
            lb_tables.setdefault((c_ip, v_ip, l_ip, s_ip, v_port, s_port), {}).setdefault("hname", hostname)
            lb_tables.setdefault((c_ip, v_ip, l_ip, s_ip, v_port, s_port), {}).setdefault("tname", table_name)
        lines = f.readline()
    return lb_tables


def node_entity_process():
    nodes_table = {}
    vm_table = {}
    edges_table, edges_infos = tcp_entity_process()
    if edges_table is None:
        print("Please wait kafka consumer datas...")
        return None, None, None, None, None
    lb_tables = lb_entity_process()
    for key in edges_table.keys():
        if len(edges_table[key]) < 2:
            continue
        dst_node_id = node_entity_name(edges_table[key]['0']['h'], edges_table[key]['0']['p'])
        src_node_id = node_entity_name(edges_table[key]['1']['h'], edges_table[key]['1']['p'])
        edge_id = edge_entity_name("tcp_link", edges_table[key]['0']['h'], edges_table[key]['0']['p'],
                                   edges_table[key]['1']['h'], edges_table[key]['1']['p'])
        edges_table.setdefault(key, {}).setdefault('src', src_node_id)
        edges_table.setdefault(key, {}).setdefault('dst', dst_node_id)
        edges_table.setdefault(key, {}).setdefault('edge', edge_id)
        #print("tcp---", key, edges_table[key])
        nodes_table.setdefault(src_node_id, {}).setdefault('host', edges_table[key]['1']['h'])
        nodes_table.setdefault(src_node_id, {}).setdefault('r_edge', [])
        nodes_table[src_node_id].get('r_edge').append((edge_id, "TCP_LINK"))
        nodes_table.setdefault(dst_node_id, {}).setdefault('host', edges_table[key]['0']['h'])
        nodes_table.setdefault(dst_node_id, {}).setdefault('l_edge', [])
        nodes_table[dst_node_id].get('l_edge').append((edge_id, "TCP_LINK"))
        if lb_tables is not None:
            for lb_key in lb_tables.keys():
                if lb_key[0] == key[0] and lb_key[1] == key[1] and lb_key[4] == key[2]:
                    lb_tables.setdefault(lb_key, {}).setdefault('src', src_node_id)
                    lb_tables.setdefault(lb_key, {}).setdefault('lb', dst_node_id)
                if lb_key[2] == key[0] and lb_key[3] == key[1] and lb_key[5] == key[2]:
                    lb_tables.setdefault(lb_key, {}).setdefault('dst', dst_node_id)

    if lb_tables is not None:
        for key in lb_tables.keys():
            lb_node_id = node_entity_name(lb_tables[key]['hname'], lb_tables[key]['tname'].split("_")[0])
            #lb_tables.setdefault(key, {}).setdefault('on', lb_node_id)
            if lb_tables.get(key, {}).get('tname') == "dnsmasq_link":
                continue
                # Add process code here....
            else:
                lb_id = edge_entity_name(lb_tables[key]['tname'], None, 
				                         lb_tables[key].get('dst'), None, lb_tables[key].get('src'))
                lb_tables.setdefault(key, {}).setdefault("lb_id", lb_id)
                nodes_table.setdefault(lb_node_id, {}).setdefault('lb_edge', [])
                nodes_table[lb_node_id].get('lb_edge').append((lb_tables[key]['lb_id'], lb_tables[key]['tname'].upper()))

    for key in nodes_table.keys():
        #print("node----", key, nodes_table[key])
        host = nodes_table[key].get('host')
        if host is None:
            print(key, "only lb link and no tcplink, please check..")
            continue
        vm_table.setdefault(host, {}).setdefault('proc', [])
        vm_table[host].get('proc').append(key)

    #for key in lb_tables.keys():
        #print("lb-----", key, lb_tables[key])

    #for key in vm_table.keys():
        #print("vm-----", key, vm_table[key])

    return edges_table, edges_infos, nodes_table, lb_tables, vm_table


def clear_tmp():
    with open(temp_tcp_file, 'wb') as file_t:
        file_t.truncate(0)
    with open(temp_other_file, 'wb') as file_o:
        file_o.truncate(0)
