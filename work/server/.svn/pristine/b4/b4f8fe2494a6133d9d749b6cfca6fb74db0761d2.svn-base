安装libnetfilter_queue。
1)即使是完全的安装，也没有libnetfilter_queue。可以使用rpm -q查询。
2)安装libnetfilter_queue-1.0.0-1.el6.x86_64.rpm


运行方式：
1) lsmod
查看是否已经装载了ip_queue模块
使用一下的命令，装载ip_queue模块：
[注意]
这里有可能已经有一个nf_queue存在了。可以使用命令查看：
cat /proc/net/netfilter/nf_queue

如没有queue，则使用如下的命令安装：
modprobe ip_queue
lsmod |grep ip_queue

2) 添加以下规则：
iptables -I INPUT -p icmp -j QUEUE
或者：
iptables -I INPUT -p icmp -j NFQUEUE

3) 运行
./ipq_user_rw

4)从xp，向192.168.19.150发送的ping测试，会被导入到QUEUE中，
然后，被ipq_user捕获、处理。
如下所示：
ipq_creat_handle success!
ipq_set_mode: send bytes =56, range=1024
recv bytes =172, nlmsg_len=172, indev=eth0, datalen=60, packet_id=78658840 
0000:  00 00 00 00 00 00 00 00 00 00 00 00 00 00 45 00 
0010:  00 3c 23 77 00 00 80 01 6e 7e c0 a8 13 e5 c0 a8 
0020:  13 96 08 00 47 5c 05 00 01 00 61 62 63 64 65 66 
0030:  67 68 69 6a 6b 6c 6d 6e 6f 70 71 72 73 74 75 76 
0040:  77 61 62 63 64 65 66 67 68 69 
Ping request from Win. Accepted!

[说明]
当前的代码中，只接受windows的icmp报文，丢弃linux的icmp报文。

//////////////////////////////////////////////////////////
新版本使用NFQUEUE and libnetfilter_queue
1)先运行：
./frep
2)然后，在iptables中添加：
# iptables -A INPUT -j NFQUEUE --queue-num 0
或，只有icmp被送到queue 0
iptables -A INPUT -p icmp -j NFQUEUE --queue-num 0
或，允许http
iptables -A FORWARD -p tcp --sport 80 -m state --state NEW,ESTABLISHED -j NFQUEUE --queue-num 0
iptables -A FORWARD -p tcp --dport 80 -m state --state NEW,ESTABLISHED -j NFQUEUE --queue-num 0
iptables -A INPUT -p tcp -m multiport --dports 80,443 -m state --state NEW,ESTABLISHED -j NFQUEUE --queue-num 0
# 另外的方法:
#针对一般的明文网站
iptables -t mangle -A FORWARD -m string --string "211.99.216." --algo bm --to 65535 -j NFQUEUE --queue-num 0
iptables -t mangle -A FORWARD -m string --string "211.99.216." --algo bm --to 65535 -j NFQUEUE --queue-balance 8:11
#针对使用gzip的网站
iptables -t mangle -A FORWARD -m string --string "Content-Encoding: gzip" --algo bm --to 65535 -j NFQUEUE --queue-num 0
#///////////// home environment
iptables -t mangle -A FORWARD -m string --string "61.149.31." --algo bm --to 65535 -j NFQUEUE --queue-num 0


# snat command
iptables -t nat -A POSTROUTING -j SNAT --to-source 192.168.55.111


将输入的信息送到NFQUEUE的queue 0中。
3)查看
# iptables --list
Chain INPUT (policy ACCEPT)
target     prot opt source               destination         
ACCEPT     udp  --  anywhere             anywhere            udp dpt:domain 
ACCEPT     tcp  --  anywhere             anywhere            tcp dpt:domain 
ACCEPT     udp  --  anywhere             anywhere            udp dpt:bootps 
ACCEPT     tcp  --  anywhere             anywhere            tcp dpt:bootps 
NFQUEUE    all  --  anywhere             anywhere            NFQUEUE num 0
4)删除规则
iptables -D INPUT 5
iptables -t mangle -D FORWARD 2


#电信局回显，route 和 iptables修改

#rc.local 
***********************************************************************
route add -net 211.99.0.0 netmask 255.255.0.0 gw 211.151.86.21
route add -net 202.108.18.0 netmask 255.255.255.0 gw 192.168.38.221
route add -net 0.0.0.0 netmask 0.0.0.0 gw 192.168.38.57
route add -net 202.108.19.0 netmask 255.255.255.0 gw  192.168.38.221
route add -net 202.108.26.0 netmask 255.255.255.0 gw 192.168.38.221
route add -net 192.168.38.52 netmask 255.255.255.252 gw 192.168.38.221


iptables -t nat -F
iptables -t nat -Z
iptables -t filter -F
iptables -t filter -Z
iptables -t mangle -F
iptables -t mangle -Z
*************************************************************************

#sysctl.conf
**************************************************************************
# Controls IP packet forwarding
net.ipv4.ip_forward = 1 

#add for firewall drop packets begin
net.nf_conntrack_max = 2097152
net.netfilter.nf_conntrack_tcp_timeout_established = 3600

# Setup DNS threshold for arp
net.ipv4.neigh.default.gc_thresh3 = 1024
net.ipv4.neigh.default.gc_thresh2 = 512
net.ipv4.neigh.default.gc_thresh1 = 128
**************************************************************************
###########################################################################
初步实现思路;
input 端口捕获所有input报文

模糊学习功能（初步打算单项参数判断，即访问次数，连接流量，访问时长，其中任何一项判断标准满足即可）
1.访问次数的更新由input报文截获，分析后负责更新
2.连接流量的更新由iptables ，connbytes 模块负责完成更新
iptables .. −m connbytes −−connbytes 10000:100000 −−connbytes−dir both −−connbytes−mode bytes ...
3.连接访问时长由 nf_conntrack 模块的  conntrack_event 检测到连接删除事件时负责更新


input入口报文截获功能
1. iptables 捕获报文入队NFQUEUE
2. 获取ip报文 payload ,判断ip报文版本，ipv6报文直接走安全口，设置允许通过
3. 获取ipv4报文，分析源IP地址，目的IP地址，源端口，目的端口，产生conn_data五元组，分析URL，
4. 检测URL白名单是否存在，如果存在且有效，则设置报文走非抗投诉出口，设置允许通过
   如果URL白名单不存在，查看IP白名单，如果存在且有效，设置报文走非抗投诉端口，设置允许通过。
   否则初始化conn_attr值，插入unordered_map   (IP头解析，TCP头解析，应用头解析，初始化函数)

iptables -t nat -F
iptables -t nat -Z
iptables -t filter -F
iptables -t filter -Z
iptables -t mangle -F
iptables -t mangle -Z
//入口设备为参数 ，假设eth0，支持多个入口，需要配多条规则
//mark 为参数
iptables -t mangle -A PREROUTING -i eth0 -j MARK --set-mark 0x1
iptables -t mangle -A PREROUTING -m mark --mark 0x1 -j NFQUEUE --queue-balance 1:16

//连接流量更新iptables
//认定>=1MB的流量为大流量，需要配置为参数
iptables -t mangle -A FORWARD -m connbytes --connbytes  1048576: --connbytes-dir reply --connbytes-mode bytes -j MARK --set-mark 0x2
iptables -t mangle -A FORWARD -m mark --mark 0x2 -j NFQUEUE --queue-balance 1:16

//报文匹配替换模块
iptables -t mangle -A FORWARD -m string --string "202.108.18." --algo bm -j MARK --set-mark 0x3
iptables -t mangle -A FORWARD -m string --string "202.108.19." --algo bm -j MARK --set-mark 0x3
iptables -t mangle -A FORWARD -m string --string "202.108.26." --algo bm -j MARK --set-mark 0x3
iptables -t mangle -A FORWARD -m mark --mark 0x3 -j NFQUEUE --queue-balance 1:16
#针对使用gzip的网站
iptables -t mangle -A FORWARD -m string --string "Content-Encoding: gzip" --algo bm -j MARK --set-mark 0x4
iptables -t mangle -A FORWARD -m mark --mark 0x4 -j NFQUEUE --queue-balance 1:16
#########################################################################################################