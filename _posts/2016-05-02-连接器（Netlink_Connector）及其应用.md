---
layout: post
title: 连接器（Netlink_Connector）及其应用
date: 2016-05-02 22:46:45
tags: C
category: C
comments: true
---


转自:[https://www.ibm.com/developerworks/cn/linux/l-connector/](https://www.ibm.com/developerworks/cn/linux/l-connector/)

<h2 id="N1003D">一、引言</h2><p>连接器是一种新的用户态与内核态的通信方式，它使用起来非常方便。本质上，连接器是一种netlink，它的 netlink 协议号为 NETLINK_CONNECTOR，与一般的 netlink 相比，它提供了更容易的使用接口，使用起来更方便。目前，最新稳定内核有两个连接器应用实例，一个是进程事件连接器，另一个是 CIFS 文件系统。连接器核心实现代码在内核源码树的driver/connector/connector.c 和 drivers/connector/cn_queue.c 文件中，文件 drivers/connector/cn_proc.c 是进程事件连接器的实现代码，而 CIFS 连接器的实现则在该文件系统的实现代码中。连接器是一个可选模块，用户可以在配置内核时在设备驱动（Device drivers）菜单中选择或不选它。</p><p>任何内核模块要想使用连接器，必须先注册一个标识 ID 和回调函数，当连接器收到 netlink 消息后，会根据消息对应的标识 ID 调用相应该 ID 的回调函数。</p><p>对用户态而言，连接器的使用跟普通的 netlink 没有差别，只要指定 netlink 协议类型为NETLINK_CONNECTOR 就可以了。</p><div  ><hr></div><p ><a href="#ibm-pcon"  >回页首</a></p><h2 id="N10048">二、连接器相关数据结构和 API</h2><p>下面是连接器的 API 以及相关的数据结构</p><p>：
</p><h5 id="N10053"><div  ><pre  >struct cb_id
{
	__u32			idx;
	__u32			val;
};
struct cn_msg
{
	struct cb_id 		id;
	__u32			seq;
	__u32			ack;
	__u32			len;		/* Length of the following data */
	__u8			data[0];
};
int cn_add_callback(struct cb_id *id, char *name, void (*callback) (void *));
void cn_del_callback(struct cb_id *id);
void cn_netlink_send(struct cn_msg *msg, u32 __group, int gfp_mask);</pre></div><p>结构 cb_id 是连接器实例的标识 ID，它用于确定 netlink 消息与回调函数的对应关系。当连接器接收到标识 ID 为 {idx，val} 的 netlink 消息时，注册的回调函数 void (*callback) (void *) 将被调用。该回调函数的参数为结构 struct cn_msg 的指针。</p><p>接口函数 cn_add_callback 用于向连接器注册新的连接器实例以及相应的回调函数，参数 id 指定注册的标识 ID，参数 name 指定连接器回调函数的符号名，参数 callback 为回调函数。</p><p>接口函数 cn_del_callback 用于卸载回调函数，参数 id 为注册函数 cn_add_callback 注册的连接器标识 ID。</p><p>接口函数 cn_netlink_send 用于向给定的组发送消息，它可以在任何上下文安全地调用。但是，如果内存不足，可能会发送失败。在具体的连接器实例中，该函数用于向用户态发送 netlink 消息。</p><p>参数 msg 为发送的 netlink 消息的消息头。参数 __group 为接收消息的组，如果它为 0，那么连接器将搜索所有注册的连接器用户，最终将发送给用户 ID 与在 msg 中的 ID 相同的组，但如果 __group 不为 0，消息将发送给 __group 指定的组。参数 gfp_mask 指定页分配标志。</p><p>注意：当注册新的回调函数时，连接器将指定它的组为 id.idx。</p><p>cn_msg 是连接器定义的消息头，字段 seq 和 ack 用于确保消息的可靠传输，刚才已经提到，netlink 在内存紧张的情况下可能丢失消息，因此该头使用顺序号和响应号来满足要求可靠传输用户的需求。当发送消息时，用户需要设置独一无二的顺序号和随机的响应号，顺序号也应当设置到 nlmsghdr-&gt;nlmsg_seq。注意 nlmsghdr 是类型为结构 struct nlmsghdr 的变量，它用于设置或保存 netlink 的消息头。每发送一个消息，顺序号应当加 1，如果需要发送响应消息，那么响应消息
的顺序号应当与被响应的消息的顺序号相同，同时响应消息的响应号应当为被响应消息的顺序号加1。如果接收到的消息的顺序号不是期望的顺序号，那表明该消息是一个新的消息，如果接收到的消息的顺序号是期望的顺序号，但它的响应号不等于上次发送消息的顺序号加1，那么它也是新消息。</p><div  ><hr></div><p ><a href="#ibm-pcon"  >回页首</a></p></h5><h2 id="N10066">三、用户态如何使用连接器</h2><p>内核 2.6.14 对 netlink 套接字有新的实现，它缺省情况下不允许用户态应用发送给组号非 1 的netlink 组，因此用户态应用要想使用非1的组，必须先加入到该组，这可以通过如下代码实现：</p><h5 id="N1006F"><div>
<pre>
 #ifndef  SOL_NETLINK
 #define  SOL_NETLINK 270
 #endif
 #ifndef  NETLINK_DROP_MEMBERSHIP
 #define  NETLINK_DROP_MEMBERSHIP 0
 #endif
 #ifndef  NETLINK_ADD_MEMBERSHIP
 #define  NETLINK_ADD_MEMBERSHIP 1
 #endif
 int group = 5;
 s = socket(PF_NETLINK, SOCK_DGRAM, NETLINK_CONNECTOR);
 l_local.nl_family = AF_NETLINK;
 l_local.nl_groups = group;
 l_local.nl_pid = getpid();
 if (bind(s, (struct sockaddr *)&amp;l_local, sizeof(struct   sockaddr_nl)) == -1) {
 	perror("bind");
 	close(s);
 	return -1;
 }
 setsockopt(s, SOL_NETLINK, NETLINK_ADD_MEMBERSHIP, &amp;group,   sizeof(group));

</pre>

</div><p>在不需要使用该连接器时使用语句</p></h5><h5 id="N10078"><div  ><pre  >setsockopt(s, SOL_NETLINK, NETLINK_DROP_MEMBERSHIP, &amp;group, sizeof(group));</pre></div><p>退出NETLINK_CONNECTOR的group组。</p><p>宏 SOL_NETLINK、NETLINK_ADD_MEMBERSHIP 和 NETLINK_DROP_MEMBERSHIP 在旧的系统中并没有定义，因此需要用户显式定义。</p><p>内核 2.6.14 的 netlink 代码只允许选择一个小于或等于最大组号的组，对于连接器，最大的组号为CN_NETLINK_USERS + 0xf， 即16，因此如果想使用更大的组号，必须修改CN_NETLINK_USERS 到该大值。增加的 0xf 个号码供非内核态用户使用。因此，组 0xffffffff目前不能使用。</p><div  ><hr></div><p ><a href="#ibm-pcon"  >回页首</a></p></h5><h2 id="N10083">四、进程事件连接器的使用</h2><p>进程事件连接器是连接器的第一个使用实例，它通过连接器来报告进程相关的事件，包括进程 fork、exec、exit 以及进程用户 ID 与组 ID 的变化。如果用户想监视系统的进程事件，就可以编一个应用程序通过 netlink 套接字来获取进程事件信息。下面将详细描述如何编写一个进程事件监视程序。</p><h5 id="N1008C"><div  >

<pre>
 #include &lt;sys/types.h&gt;
 #include &lt;sys/socket.h&gt;
 #include &lt;signal.h&gt;
 #include &lt;linux/netlink.h&gt;
 #include &lt;linux/connector.h&gt;
 #define _LINUX_TIME_H
 #include &lt;linux/cn_proc.h&gt;
 </pre>
 
 </div><p>上面这些 include 语句包含了进程监视程序需要的必要头文件，其中头文件 sys/types.h 和sys/socket.h 是编写套接字程序所必须的，头文件 signal.h 包含了信号处理相关的函数，本程序需要信号处理，因此需要包含该头文件。其余的三个头文件是内核相关的头文件，头文件linux/netlink.h 是编写netlink套接字程序所必须的，头文件 linux/connector.h 包含了内核实现的连接器的一些结构和宏，使用连接器监视系统事件的程序必须包含它，头文件 linux/cn_proc.h 则定义了进程事件连接器的一些结构和宏，应用程序需要包含该头文件以便正确分析进程事件。注意，在包含头文件 linux/cn_proc.h 之前定义了宏_LINUX_TIME_H，因为在用户态应用中包含linux/time.h会导致结构struct timespec 定义冲突，所以该宏避免了头文件linux/cn_proc.h包含linux/time.h。</p></h5><h5 id="N10095"><div  ><pre  >#define  MAX_MSGSIZE 256
#ifndef  SOL_NETLINK
#define  SOL_NETLINK 270
#endif</pre></div><p>旧的系统并没有定义 SOL_NETLINK，因此程序必须处理这种情况。宏 MAX_MSGSIZE 定义了最大的进程事件消息大小，它用于指定接收进程事件消息的缓存的大小，这里只是很粗略的大小，实际的消息比这小。</p></h5><h5 id="N1009E"><div  ><pre  >int sd;
struct sockaddr_nl l_local, daddr;
int on;
int len;
struct nlmsghdr *nlhdr = NULL;
struct msghdr msg;
struct iovec iov;
int * connector_mode;
struct cn_msg * cnmsg;
struct proc_event * procevent;
int counter = 0;
int ret;
struct sigaction sigint_action;</pre></div><p>这些变量用于处理 netlink 消息，其中 sd 为套接字描述符，l_local 和 daddr 分别表示 netlink消息的源地址和目的地址，后面部分将详细解释这种地址的设置。</p></h5><h5 id="N100A7"><div  ><pre  >void change_cn_proc_mode(int mode)</pre></div><p>函数 change_cn_proc_mode 用于打开和关闭进程事件的报告，进程事件连接器初始化时是关闭进程事件报告的，一个进程要想监视进程事件，必须首先打开进程事件连接器的报告开关，在它退出是必须关闭进程事件连接器的报告开关，否则进程事件连接器将继续报告进程事件，尽管没有一个监视进程对这些事件感兴趣，这将造成不必要的系统开销，同时因为缓存这些事件浪费了宝贵的系统内存。下面代码是该函数的实现：</p></h5><h5 id="N100B0"><div  ><pre  >{
	memset(nlhdr, 0, sizeof(NLMSG_SPACE(MAX_MSGSIZE)));
	memset(&amp;iov, 0, sizeof(struct iovec));
	memset(&amp;msg, 0, sizeof(struct msghdr));
        cnmsg = (struct cn_msg *)NLMSG_DATA(nlhdr);
        connector_mode = (int *)cnmsg-&gt;data;
        * connector_mode = mode;
        nlhdr-&gt;nlmsg_len = NLMSG_LENGTH(sizeof(struct cn_msg) + sizeof(enum
		proc_cn_mcast_op));
        nlhdr-&gt;nlmsg_pid = getpid();
        nlhdr-&gt;nlmsg_flags = 0;
        nlhdr-&gt;nlmsg_type = NLMSG_DONE;
        nlhdr-&gt;nlmsg_seq = 0;</pre></div><p>对于进程事件连接器，netlink 消息包括 netlink 消息头、连接器消息头、进程事件或控制操作指令，其中进程事件或控制操作指令部分是变长的，如果是控制指令，仅包含4个字节，如果是进程事件，它应当为类型 struct proc_event 的结构，对于不同的事件，尺寸不同，可能的事件包括控制指令的应答、进程 fork、进程 exec、进程 exit、进程用户 ID 改变以及进程组 ID 的改变。变量 connector_mode 用于设置控制指令，对于进程事件连接器，只有两种控制指令，分别是PROC_CN_MCAST_LISTEN 和 PROC_CN_MCAST_IGNORE，对应于打开和关闭进程事件报告。这两个宏定义在头文件 linux/cn_proc.h。变量 nlhdr 用于设置 netlink 的消息头，nlmsg_len用于指明消息的数据部分长度，该消息的数据部分包含了固定长度的连接器的消息头以及进程连接器的消息，nlmsg_pid用于指定消息的来源，一般为进程或线程ID，nlmsg_flags用于指定一些特殊标志，一般设置为0就足够了。应用程序设置 nlmsg_type 为 NLMSG_DONE，表示该消息是完整的，没有后续的消息碎片。一般地，nlmsg_seq 应当与连接器消息头的顺序号一致。</p></h5><h5 id="N100B9"><div  ><pre  >        cnmsg-&gt;id.idx = CN_IDX_PROC;
        cnmsg-&gt;id.val = CN_VAL_PROC;
        cnmsg-&gt;seq = 0;
        cnmsg-&gt;ack = 0;
        cnmsg-&gt;len = sizeof(enum proc_cn_mcast_op);</pre></div><p>这部分代码用于设置连接器消息头，对于进程事件连接器，cnmsg-&gt;id.idx 和 cnmsg-&gt;id.val 必须分别设置为CN_IDX_PROC和CN_VAL_PROC，否则该消息无法派送给进程事件连接器。Seq 和ack 用于指定消息的顺序号和响应号，对于非响应消息，ack 应当设置为 0，而顺序号应当为上一个发送的消息的顺序号加1，对于第一个消息可以随意指定顺序号。</p></h5><h5 id="N100C2"><div  ><pre  >        iov.iov_base = (void *)nlhdr;
        iov.iov_len = nlhdr-&gt;nlmsg_len;
        msg.msg_name = (void *)&amp;daddr;
        msg.msg_namelen = sizeof(daddr);
        msg.msg_iov = &amp;iov;
        msg.msg_iovlen = 1;
        ret = sendmsg(sd, &amp;msg, 0);
        if (ret == -1) {
        	perror("sendmsg error:");
		exit(-1);
        }
}</pre></div><p>这部分代码用于发送 netlink 消息，为了通过函数 sendmsg 发送该消息，程序必须填写类型为结构 struct msghdr 的变量 msg，因为该函数可以一次发送多个消息，因此通过结构 struct iovec 来组织所有要发送的消息。iov.iov_base 指向消息的开始位置，iov.iov_len 指定消息的大小，msg.msg_name 指定消息的目的地址，msg.msg_namelen 则指定消息的目的地址长度，msg.msg_iov 指向结构为 struct iovec 的数组开始位置，对于这里的情况，它只包含了一个元素，因此 msg.msg_iovlen 设置为 1，如果有多个消息，该字段应该设置为实际的消息数，当然那时 iov 应当是一个多元素的数组，每一个元素都应当象前面的 iov 结构去设置。</p></h5><h5 id="N100CB"><div  ><pre  >void sigint_handler(int signo)
{
	change_cn_proc_mode(PROC_CN_MCAST_IGNORE);
	printf("process event: turn off process event listening.\n");
	close(sd);
	exit(0);
}</pre></div><p>这是一个信号处理函数，它用于在该程序退出时关闭进程事件的报告。</p><p>下面是程序的主体部分。</p></h5><h5 id="N100D6"><div  ><pre  >int main(void)
{
	memset(&amp;sigint_action, 0, sizeof(struct sigaction));
	sigint_action.sa_flags = SA_ONESHOT;
	sigint_action.sa_handler = &amp;sigint_handler;
	sigaction(SIGINT, &amp;sigint_action, NULL);</pre></div><p>这段代码用于设置信号 SIGINT 的处理函数，该程序是一个无限循环，用户通过 CTRL + C 来退出，当用户按下 CTRL + C 时，系统将发送信号 SIGINT 该该程序，相应的处理函数将被执行，前面已经讲过，该信号处理函数用于关闭进程事件报告。</p></h5><h5 id="N100DF"><div  ><pre  >	nlhdr = (struct nlmsghdr *)malloc(NLMSG_SPACE(MAX_MSGSIZE));
	if (nlhdr == NULL) {
		perror("malloc:");
		exit(-1);
	}
	
daddr.nl_family = AF_NETLINK;
daddr.nl_pid = 0;
daddr.nl_groups = CN_IDX_PROC;</pre></div><p>netlink 消息的地址结构包括三个主要的字段，nl_family 必须设置为 AF_NETLINK，nl_pid 则用于指定 netlink 消息的接收者或发送者的地址，一般为进程 ID 或线程 ID，如果该消息的发送者为内核或接收者有多个，它设置为 0，此时 nl_groups 指定接收者的组号。</p></h5><h5 id="N100E8"><div  ><pre  >		sd = socket(PF_NETLINK, SOCK_DGRAM, NETLINK_CONNECTOR);</pre></div><p>该语句创建了一个 netlink 套接字，注意对于使用连接器的应用，第三个参数必须指定为 NETLINK_CONNECTOR。所有使用 netlink 的应用程序，函数 socket 的前两个参数都是一样的，应当分别为 PF_NETLINK 和 SOCK_DGRAM。</p></h5><h5 id="N100F1"><div  ><pre  >	l_local.nl_family = AF_NETLINK;
	l_local.nl_groups = CN_IDX_PROC;
	l_local.nl_pid = getpid();</pre></div><p>这段代码用于设置 netlink 消息的源地址。</p></h5><h5 id="N100FA"><div  ><pre  >	if (bind(sd, (struct sockaddr *)&amp;l_local, sizeof(struct sockaddr_nl)) == -1)
	{
        	perror("bind");
	        close(sd);
        	return -1;
	}</pre></div><p>使用 bind 函数主要为了把源地址与套接字 sd 绑定起来，以便后面消息的发送不必指定源地址。</p></h5><h5 id="N10103"><div  ><pre  >		change_cn_proc_mode(PROC_CN_MCAST_LISTEN);</pre></div><p>该调用打开了进程事件的报告。</p></h5><h5 id="N1010C"><div  ><pre  >	printf("process event: turn on process event listening.\n");
	while (1) {
		memset(nlhdr, 0, NLMSG_SPACE(MAX_MSGSIZE));
		memset(&amp;iov, 0, sizeof(struct iovec));
		memset(&amp;msg, 0, sizeof(struct msghdr));
                iov.iov_base = (void *)nlhdr;
                iov.iov_len = NLMSG_SPACE(MAX_MSGSIZE);
                msg.msg_name = (void *)&amp;daddr;
                msg.msg_namelen = sizeof(daddr);
                msg.msg_iov = &amp;iov;
                msg.msg_iovlen = 1;
                ret = recvmsg(sd, &amp;msg, 0);
                if (ret == 0) {
                        printf("Exit.\n");
                        exit(0);
                }
                else if (ret == -1) {
                        perror("recvmsg:");
                        exit(1);
                }
		else {</pre></div><p>这部分代码用于接收进程事件消息，使用函数 recvmsg 时，用户也必须设置 msg，这时各字段的意义与发送时不一样，iov 用于指定消息的存放位置以及最大可利用的缓存大小，msg.msgname 则表示该调用希望接收的消息的目的地址，msg.msg_iovlen 则指定该调用应当返回的消息数。</p></h5><h5 id="N10115"><div  ><pre  >cnmsg = (struct cn_msg *)NLMSG_DATA(nlhdr);
procevent = (struct proc_event *)cnmsg-&gt;data;
switch (procevent-&gt;what) {
  case PROC_EVENT_NONE:
    printf("process event: acknowledge for turning on process
    event listening\n\n\n");
    break;
  case PROC_EVENT_FORK:
    printf("process event: fork\n");
    printf("parent tid:%d, pid:%d\nchild tid:%d, pid:%d\n\n\n",
     procevent-&gt;event_data.fork.parent_pid,
     procevent-&gt;event_data.fork.parent_tgid,
     procevent-&gt;event_data.fork.child_pid,
     procevent-&gt;event_data.fork.child_tgid);
    break;
  case PROC_EVENT_EXEC:
    printf("process event: exec\n");
    printf("tid:%d, pid:%d\n\n\n",
     procevent-&gt;event_data.exec.process_pid,
     procevent-&gt;event_data.exec.process_tgid);
    break;
  case PROC_EVENT_UID:
    printf("process event: uid\n");
    printf("process tid:%d, pid:%d, uid:%d-&gt;%d\n\n\n",
     procevent-&gt;event_data.id.process_pid,
     procevent-&gt;event_data.id.process_tgid,
     procevent-&gt;event_data.id.r.ruid,
     procevent-&gt;event_data.id.e.euid);
    break;
  case PROC_EVENT_GID:
    printf("process event: gid\n");
    printf("process tid:%d, pid:%d, uid:%d-&gt;%d\n\n\n",
     procevent-&gt;event_data.id.process_pid,
     procevent-&gt;event_data.id.process_tgid,
     procevent-&gt;event_data.id.r.rgid,
     procevent-&gt;event_data.id.e.egid);
    break;
  case PROC_EVENT_EXIT:
    printf("process event: exit\n");
    printf("tid:%d, pid:%d, exit code:%d\n\n\n",
     procevent-&gt;event_data.exit.process_pid,
     procevent-&gt;event_data.exit.process_tgid,
     procevent-&gt;event_data.exit.exit_code);
    break;
  default:
    printf("Unkown process action\n\n\n");
    break;
}
    }
  }
}</pre></div><p>这部分代码用于处理各种不同的进程事件，并输出具体的事件信息，对于 fork 事件，输出父进程和线程的 ID 以及子进程和线程的 ID，对于 exec 事件则输出执行 exec 调用的进程和线程的 ID，对于用户 ID 变更事件，则输出制造该事件的进程和线程的 ID，旧的用户 ID 以及新的用户 ID，对于组 ID 变更事件，则输出制造该事件的进程和线程的 ID，旧的组 ID 以及新的组 ID，对于 exit 事件，则输出结束运行的进程和线程的 ID 以及退出码。</p><p>下面是该程序在作者的红旗 Linux 桌面版 4.1 上的运行结果示例：</p></h5><h5 id="N10120"><div  ><pre  >[root@localhost yangyi]# gcc -I linux-2.6.15.4/include cn_proc_user.c -o
cn_proc_user
[root@localhost yangyi]# ./cn_proc_user
process event: turn on process event listening.
process event: acknowledge for turning on process event listening
process event: fork
parent tid:2720, pid:2720
child tid:2775, pid:2775
process event: exec
tid:2775, pid:2775
process event: exit
tid:2775, pid:2775, exit code:0
.
.
.
process event: uid
process tid:2877, pid:2877, uid:500-&gt;0
process event: gid
process tid:2877, pid:2877, gid:500-&gt;500
process event: uid
process tid:2877, pid:2877, uid:500-&gt;0
process event: uid
process tid:2877, pid:2877, uid:500-&gt;0
process event: uid
process tid:2877, pid:2877, uid:500-&gt;0
process event: uid
process tid:2877, pid:2877, uid:500-&gt;0
process event: fork
parent tid:2877, pid:2877
child tid:2878, pid:2878
process event: gid
process tid:2878, pid:2878, gid:500-&gt;500
process event: uid
process tid:2878, pid:2878, uid:500-&gt;500
process event: exec
tid:2878, pid:2878
process event: exit
tid:2878, pid:2878, exit code:0
process event: turn off process event listening.
[root@localhost yangyi]#</pre></div><div  ><hr></div><p ><a href="#ibm-pcon"  >回页首</a></p></h5><h2 id="N10125">五、如何实现一个新的连接器实例</h2><p>要想实现一个新的连接器，必须首先定义个新的连接器标识，目前最新的内核包括两个连接器实例，一个是进程事件连接器，另一个为 CIFS 连接器，因此新的连接器标识必须不同于现有的任何一个连接器标识。例如，用户可以使用如下语句来定义一个新的连接器标识：</p><h5 id="N1012E"><div  >
<pre  >#define CN_IDX_NEW 3
#define CN_VAL_NEW 1
</pre>
</div><p>当然连接器必须在内核实现，因此需要通过内核模块来定义相应的回调函数并在初始化代码中注册该回调函数，回调函数实际上用于处理发送给该连接器的消息。该模块也必须实现消息发送函数供其它内核子系统方便使用该连接器。下面是作者编写的一个文件系统事件连接器的实现代码，该代码根据进程事件连接器（drivers/connector/cn_proc.c）编写而成。</p><p>头文件 include/linux/cn_fs.h 定义了文件系统事件处理的数据结构、open 消息发送函数声明以及一些相关的宏定义，结构 struct fs_event 定义了文件系统事件连接器消息结构。</p></h5><h5 id="N10139"><div>

<pre>
 #ifndef CN_FS_H
 #define CN_FS_H 
 #include &lt;linux/types.h&gt; 
 #include &lt;linux/time.h&gt; 
 #include &lt;linux/connector.h&gt; 
 #define TASK_NAME_LEN 16 
 #define FILE_NAME_LEN 256 
 #define CN_IDX_FS 3 
 #define CN_VAL_FS 1 
 /*
 * Userspace sends this enum to register with the kernel that it is listening
 * for events on the connector.
 */
enum fs_cn_mcast_op {
	FS_CN_MCAST_LISTEN = 1,
	FS_CN_MCAST_IGNORE = 2
};
struct fs_event {
	enum type {
		/* Use successive bits so the enums can be used to record
		 * sets of events as well
		 */
		FS_EVENT_NONE = 0x00000000,
		FS_EVENT_OPEN = 0x00000001
	} type;
	__u32 cpu;
	struct timespec timestamp;
	union {
		struct {
			__u32 err;
		} ack;
		struct fs_read_event {
			char proc_name[TASK_NAME_LEN];
			char file_name[FILE_NAME_LEN];
		} read;
	} event_data;
};
 
 #ifdef __KERNEL__ 

 #ifdef CONFIG_FS_EVENTS 
 void fs_open_connector(struct dentry * dentryp); 

 #else 

 static void fs_open_connector(struct dentry * dentryp) 

 {} 
 #endif	/* CONFIG_FS_EVENTS */ 
 #endif	/* __KERNEL__ */ 
 #endif	/* CN_FS_H */ 
 </pre>
 </div><p>下面文件 drivers/connector/cn_fs.c 是文件系统连接器的实现代码。</p></h5><h5 id="N10142"><div  ><pre  >#include &lt;linux/module.h&gt;
 #include &lt;linux/kernel.h&gt; 
 #include &lt;linux/init.h&gt; 
 #include &lt;linux/fs.h&gt; 
 #include &lt;linux/cn_fs.h&gt; 
 #include &lt;asm/atomic.h&gt; 
</pre></div><p>这些是必要的内核头文件。</p></h5><h5 id="N1014B"><div  ><pre  >#define CN_FS_MSG_SIZE (sizeof(struct cn_msg) + sizeof(struct fs_event))</pre></div><p>该宏定义了文件系统消息的大小。</p></h5><h5 id="N10154"><div  ><pre  >static atomic_t fs_event_listeners = ATOMIC_INIT(0);</pre></div><p>该变量用于控制文件系统 open 事件的报告，初始化时设置为 0，即不报告 open 事件。用户态应用可以通过向文件系统连接器发送控制消息来打开和关闭 open 事件的报告。</p><p>static struct cb_id cn_fs_event_id = { CN_IDX_FS, CN_VAL_FS };
这是连接器的唯一标识，连接器需要它来找到对应的连接器实例。</p></h5><h5 id="N1015F"><div  ><pre  >/* fs_event_counts is used as the sequence number of the netlink message */
static DEFINE_PER_CPU(__u32, fs_event_counts) = { 0 };</pre></div><p>该 PER_CPU 变量用于统计总共的文件系统事件，并通过它来获得连接器消息的顺序号。</p></h5><h5 id="N10168"><div  ><pre  >static inline void get_seq(__u32 *ts, int *cpu)
{
	*ts = get_cpu_var(fs_event_counts)++;
	*cpu = smp_processor_id();
	put_cpu_var(fs_event_counts);
}</pre></div><p>该函数用于得到下一个消息的顺序号。</p></h5><h5 id="N10171"><div  ><pre  >void fs_open_connector(struct dentry * dentryp)
{
	struct cn_msg *msg;
	struct fs_event *event;
	__u8 buffer[CN_FS_MSG_SIZE];
	if (atomic_read(&amp;fs_event_listeners) &lt; 1)
		return;
	printk("cn_fs: fs_open_connector\n");
	msg = (struct cn_msg*)buffer;
	event = (struct fs_event*)msg-&gt;data;
	get_seq(&amp;msg-&gt;seq, &amp;event-&gt;cpu);
	getnstimestamp(&amp;event-&gt;timestamp);
	event-&gt;type = FS_EVENT_OPEN;
	memcpy(event-&gt;event_data.read.proc_name, current-&gt;comm,
	TASK_NAME_LEN);
	memcpy(event-&gt;event_data.read.file_name, dentryp-&gt;d_name.name,
	dentryp-&gt;d_name.len);
	event-&gt;event_data.read.file_name[dentryp-&gt;d_name.len] = '\0';
	memcpy(&amp;msg-&gt;id, &amp;cn_fs_event_id, sizeof(msg-&gt;id));
	msg-&gt;ack = 0; /* not used */
	msg-&gt;len = sizeof(struct fs_event);
	/*  If cn_netlink_send() failed, the data is not sent */
	cn_netlink_send(msg, CN_IDX_FS, GFP_KERNEL);
}</pre></div><p>该函数为 open 事件消息的发送函数，它被文件系统的 open 操作调用来向文件系统事件连接器发送 open 事件。它首先设置文件系统事件结构 struct fs_event 的各个字段，字段event-&gt;timestamp为发生事件的时间，event-&gt;type为事件的类型，该模块只实现了两个事件，一个为对控制操作的响应，另一个为 open 事件。字段event-&gt;event_data.read.proc_name为打开文件的进程名称，event-&gt;event_data.read.file_name 则为被打开的文件名。消息设置完毕后可通过连接器接口函数 cn_netlink_send 直接发送，该发送函数不能保证消息发送成功，因此对于要求可靠传输消息的应用，必须通过响应来最终确认是否发送成功。</p></h5><h5 id="N1017A"><div  ><pre  >static void cn_fs_ack(int err, int rcvd_seq, int rcvd_ack)
{
	struct cn_msg *msg;
	struct fs_event *event;
	__u8 buffer[CN_FS_MSG_SIZE];
	if (atomic_read(&amp;fs_event_listeners) &lt; 1)
		return;
	msg = (struct cn_msg*)buffer;
	event = (struct fs_event*)msg-&gt;data;
	msg-&gt;seq = rcvd_seq;
	getnstimestamp(&amp;event-&gt;timestamp);
	event-&gt;cpu = -1;
	event-&gt;type = FS_EVENT_NONE;
	event-&gt;event_data.ack.err = err;
	memcpy(&amp;msg-&gt;id, &amp;cn_fs_event_id, sizeof(msg-&gt;id));
	msg-&gt;ack = rcvd_ack + 1;
	msg-&gt;len = sizeof(struct fs_event);
	cn_netlink_send(msg, CN_IDX_FS, GFP_KERNEL);
}</pre></div><p>该函数用于给用户态发送响应消息。注意，响应消息的顺序号必须为被响应的消息的顺序号，响应号则为顺序号加1。</p></h5><h5 id="N10183"><div  ><pre  >static void cn_fs_mcast_ctl(void *data)
{
	struct cn_msg *msg = data;
	enum fs_cn_mcast_op *mc_op = NULL;
	int err = 0;
	if (msg-&gt;len != sizeof(*mc_op))
		return;
	mc_op = (enum fs_cn_mcast_op*)msg-&gt;data;
	switch (*mc_op) {
	case FS_CN_MCAST_LISTEN:
		atomic_inc(&amp;fs_event_listeners);
		break;
	case FS_CN_MCAST_IGNORE:
		atomic_dec(&amp;fs_event_listeners);
		break;
	default:
		err = EINVAL;
		break;
	}
	cn_fs_ack(err, msg-&gt;seq, msg-&gt;ack);
}</pre></div><p>该函数为注册给连接器的回调函数，它用于处理用户态应用发送给该连接器的消息。因此，实际上它是消息接收函数。对于该模块，它实际上用于处理控制命令，用户态发送的控制命令消息最后将由它来处理，它实际上用于打开和关闭文件系统事件报告开关，同时它也负责发送响应消息给用户态应用。</p></h5><h5 id="N1018C"><div  ><pre  >static int __init cn_fs_init(void)
{
	int err;
	if ((err = cn_add_callback(&amp;cn_fs_event_id, "cn_fs",
	 			   &amp;cn_fs_mcast_ctl))) {
		printk(KERN_WARNING "cn_fs failed to register\n");
		return err;
	}
	return 0;
}</pre></div><p>该函数在内核初始化时调用，它使用连接器接口函数 cn_add_callback 注册了一个新的连接器实例。</p></h5><h5 id="N10195"><div  ><pre  >module_init(cn_fs_init);</pre></div><p>该语句用于告诉内核函数 cn_fs_init 需要在内核初始化时调用。</p><p>程序源码包中的 cn_fs_user.c 是使用该文件系统连接器来监视文件系统 open 事件的一个示例程序，它的大部分代码与前面的进程事件监视程序示例一样，只是把进程事件相关的处理部分替换为文件系统事件对应处理。另外一点需要特别注意，对于组号大于 1 的连接器，用户态应用必须通过第三节介绍的方式来加入到组中，否则，应用无法收到连接器的消息。</p><p>下面是作者在红旗 Linux 桌面版 4.1 上运行 cn_fs_user 的输出结果示例：</p></h5><h5 id="N101A2"><div  ><pre  >[root@localhost yangyi]# gcc -I linux-2.6.14.5/include cn_fs_user.c -o
cn_fs_user
[root@localhost yangyi]# ./cn_fs_user
filesystem event: turn on filesystem event listening.
filesystem event: acknowledge for turning on filesystem event listening
filesystem event: open
process 'rfdock' open file 'en.xpm'
filesystem event: open
process 'bash' open file 'passwd'
filesystem event: open
process 'cat' open file 'ld.so.cache'
filesystem event: open
process 'cat' open file 'libc-2.3.2.so'
filesystem event: open
process 'cat' open file 'locale-archive'
filesystem event: open
process 'cat' open file 'test_cn_proc.c'
filesystem event: open
process 'rfdock' open file 'en.xpm'
filesystem event: turn off filesystem event listening.
[root@localhost yangyi]#</pre></div><div  ><hr></div><p ><a href="#ibm-pcon"  >回页首</a></p></h5><h2 id="N101A7">小结</h2><p>连接器是非常便利的用户态与内核态的通信方式，内核开发者在编写内核子系统或模块时可以采用这种方式方便地进行用户态与内核态的数据交换。本文详细地讲解了连接器的使用，读者通过阅读本文应当掌握连接器的功能和使用。想了解其它用户态与内核态通信方式的读者，可以参考作者以前的系列文章在 "Linux 下用户空间与内核空间数据交换的方式，<a href="http://www.ibm.com/developerworks/cn/linux/l-kerns-usrs/">第 1 部分</a>"和"<a href="http://www.ibm.com/developerworks/cn/linux/l-kerns-usrs2/">第 2 部分</a>"。</p>

<h2 id="resources"  >参考资料 </h2><ol><li>内核文档，Documentation/connector.txt。</li><li>内核源代码，Linux 2.6.15.4。</li><li>在Linux下用户空间与内核空间数据交换的方式，第1部分，
<a href="http://www.ibm.com/developerworks/cn/linux/l-kerns-usrs/">http://www.ibm.com/developerworks/cn/linux/l-kerns-usrs/</a></li><li>在 Linux 下用户空间与内核空间数据交换的方式，第 2部分，
<a href="http://www.ibm.com/developerworks/cn/linux/l-kerns-usrs2/">http://www.ibm.com/developerworks/cn/linux/l-kerns-usrs2/</a></li></ol>
