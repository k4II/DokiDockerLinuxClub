使用源码前，请先解压rootfs文件到rootfs文件夹。。。。。

明白了一些东西，原来root的权限也会被限制，也有即使是root用户也无法操作的文件，也遇到了很多问题，无法解决，只能自己经过各种尝试进行盲目猜测。。。。

sys/capability.h需要安装libcap-devel开发库，使用sudo apt install libcap-dev命令就可以了（注：对，就是libcap-dev不是libcap-devel（不知道有什么用，找的教程里有这个，但是代码也不需要这个库。。暂备着

没完成：uid能映射出来，gid映射不出来，gid也显示的nogroup，不知道为什么。。。

rootfs虽然是挂载成功了，但是还是存在疑惑，文件系统，权限，一言难尽。。。。

## 参考教程

其实别人已经说的很清楚了，我复制粘贴也没什么意义。。。

[DOCKER基础技术：LINUX NAMESPACE（上）](https://coolshell.cn/articles/17010.html)

[DOCKER基础技术：LINUX NAMESPACE（下）](https://coolshell.cn/articles/17029.html)

[Docker资源隔离和限制实现原理](http://lionheartwang.github.io/blog/2018/03/18/dockerzi-yuan-ge-chi-he-xian-zhi-shi-xian-yuan-li/)

[namespace资源隔离](https://www.ihaiyun.cc/2018/07/21/Docker-namespace/)

## 遇到的问题和各种盲目猜测

挂载 /proc文件系统 后，使用sudo进入“容器”，退出"容器"后，sudo命令无法使用，报错sudo: no tty present and no askpass program specified

看到大概相关的解决办法也觉得好像不是太一样，也看不大懂。。。

> 在chroot环境中，这些其他答案可能无法正常工作......可能是因为：
>
> 1. / etc / shadow vs / etc / passwd冲突不允许用户输入密码。
> 2. 在chroot-ed环境中，访问tty1可能有点毛病，而ctrl-alt f2 - to tty2是不可行的，因为它是非chroot-ed环境的tty。
>
> **例如：***使用chroot环境（例如Archlinux和arch-chroot）手动安装/修复linux或bootloader。*



使用ps命令，会提示Error, do this: mount -t proc proc /proc 挂载又需要root权限，然而sudo不能用了。。。

话说隔离了应该对“外部”没有影响才对，没有完全隔离开？或者说隔离开了，但是exit“容器”过后，已经被挂载的东西还挂在namespace里面，所以“外部”受到了影响，从“外部”无法访问到被隔离的挂载文件？

umount并不能取消挂载，甚至报错找不到文件，但是cd命令又能够打开，里面的文件也少了很多，是里面的文件被隔离了？而不能取消挂载的原因是”外部“也并不能操作namespace里的文件？![](https://s2.ax1x.com/2019/06/14/V54s6x.png)

Ubuntu默认挂载/proc文件系统，所以使用ps命令的时候不需要再挂载一遍，但是使用了Linux namespace，mount了的文件系统被隔离，所以”外部“使用ps命令的时候访问不到命令需要的/proc文件系统，才会出现Error, do this: mount -t proc proc /proc提示？sudo无法使用可能也是因为这个，需要的文件被隔离了？

关掉进程隔离，我们可以通过pstree命令看见进程的父子关系![](https://s2.ax1x.com/2019/06/15/VI9UHg.png)

系统启动时，内核将创建一个默认的PID Namespace，该Namespace是所有以后创建的Namespace的祖先，因此系统所有的进程在该Namespace都是可见的。因此在我开启了多个namespace，没有开pid namespace，理论上各个namespace内的进程都是可见的，只有一个子进程，说明只有一个namespace，这个namespace兼有各个namespace的功能。

clone()建立的新进程就是调用的那个bash，那么namespace里面的进程就是这个bash，当一个namespace中的所有进程都退出时，该namespace将会被销毁。有方法能让namespace一直存在，以 ipc namespace为例：

> 通过mount --bind命令。例如mount --bind /proc/1000/ns/ipc /other/file，就算属于这个ipc namespace的所有进程都退出了，只要/other/file还在，这个ipc namespace就一直存在，其他进程就可以利用/other/file，通过setns函数加入到这个namespace

所以我猜测那么namespace并没有被销毁，通过挂载一些无关文件，查看挂载信息，这个namespace确实没有被销毁，在bash已经退出的情况下，挂载依然还存在。

但是在设置完用户隔离后，namespace会被销毁，为什么，用户的隔离为什么会影响到文件系统？？

### 关于user namespace的疑问

教程上说如果要把user namespace与其他namespace混合使用，那么依旧需要root权限。解决方案是以普通用户身份创建user namespace，然后在新建的namespace中作为root，在clone()进程加入其他类型的namespace隔离。但是实际操作上，我并没有提权，直接以普通用户执行，其他的namespace依然能够成功？？？调用clone()创建的namespace不是互相独立的，他们是同一个namespace，以普通用户创建user namespace，也应该是用普通用户创建的其他namespace，但是其他namespace需要root权限，为什么会成功呢？

#### 为什么user namespace里的用户会出现权限问题？

以下皆为盲目猜测：以普通用户创建user namespace，普通用户成为了这个namespace里面的root用户，只有mount的文件系统才属于这个namespace里的用户，root也只针对挂载的文件系统，没有进行挂载的文件属于共享，但是namespace里面的用户对它并没有root权限。

举个例子，小明买了个扫把，小明怎么使用这个扫把，甚至扔掉都没关系，因为这个扫把是小明自己的，但是，一天，小明把这个扫把拿出来和室友一起用，室友可以使用这个扫把，但是室友没权利把这个扫把扔掉，因为这个扫把只是小明拿出来分享，室友有使用权并没有处置权。小明就是root用户，室友是普通用户，扫把就是那个没有进行挂载的文件。而室友能处置只有自己的东西，也就是挂载的文件。

但是问题来了，我使用root用户运行代码也出现了权限问题，是因为就算我是用的root用户创建的namespace，但是对于namespace里面的这个进程bash，这个用户是namespace里面的用户，跟创建这个namespace的用户无关，这个用户会体现只针对namespace里面有root权限，而对外是普通用户的特点？？

但是以上猜测还是有问题，mount命令也需要root权限，但是代码中mount命令能够执行，apt update就不行，user namespace的root权限范围有多大？

> ```
> 拥有一个能力user namespace允许进程对其执行特权操作由（非用户）命名空间管理的资源（与之关联）用户名称空间（请参阅下一小节）。
> 另一方面，有许多特权操作会影响与任何名称空间类型无关的资源例如，更改系统时间（由CAP_SYS_TIME控制），加载内核模块（由CAP_SYS_MODULE控制），并创建一个设备（由CAP_MKNOD管理）。只有具有初始用户命名空间特权的进程才能执行此类操作。（来自man7谷歌翻译）
> ```

#### 关于挂载

因为不知道怎么挂载rootfs那个文件系统，我把它转成了光盘镜像iso文件，然后再挂载，但是在代码中mount -o loop -t iso9660 ~/rootfs.iso ~/mnt 会报错mount failed:Operation not permitted.但是单独用命令行或者单独开一个c文件来运行，只要开了root就没问题。这么说来，应该是有关namespace的问题。最后通过mount --bind方法挂载上去了，虽然挂载信息上看起来怪怪的，但是挂载点里面确实有文件，但是原问题并没有解决，为什么不能操作呢，也没有 i 限制啊？（本想chroot直接隔离，然而觉得Linux namespace为题应该用挂载来隔离。。。然后。。。是我自己作死。。。）

诡异的是，perror函数是输出上一个函数的错误原因，他输出了success。。但我挂载明显出错了。。

古怪，网络隔离撤掉后，sys挂载不了了。。。。

#### 关于网络

哎，网络这东西发现我真的是一窍不通。。。

从“外部”ping“容器”内部的网卡ip能ping通，但是从“容器”也能ping通“外部”的ens33网卡的ip，但不能ping通其他的ip，只能发包，没有回复。尝试过桥接和NAT，结果都是一样，以下是桥接参考：

> 首先，我们先增加一个网桥lxcbr0，模仿docker0
>
> brctl addbr lxcbr0
> brctl stp lxcbr0 off
> ifconfig lxcbr0 192.168.10.1/24 up #为网桥设置IP地址
>
> 接下来，我们要创建一个network namespace - ns1
>
> 增加一个namesapce 命令为 ns1 （使用ip netns add命令）
>
> ip netns add ns1 
>
> 激活namespace中的loopback，即127.0.0.1（使用ip netns exec ns1来操作ns1中的命令）
>
> ip netns exec ns1   ip link set dev lo up 
>
> 然后，我们需要增加一对虚拟网卡
>
> 增加一个pair虚拟网卡，注意其中的veth类型，其中一个网卡要按进容器中
>
> ip link add veth-ns1 type veth peer name lxcbr0.1
>
> 把 veth-ns1 按到namespace ns1中，这样容器中就会有一个新的网卡了
>
> ip link set veth-ns1 netns ns1
>
> 把容器里的 veth-ns1改名为 eth0 （容器外会冲突，容器内就不会了）
>
> ip netns exec ns1  ip link set dev veth-ns1 name eth0 
>
> 为容器中的网卡分配一个IP地址，并激活它
>
> ip netns exec ns1 ifconfig eth0 192.168.10.11/24 up
>
> 上面我们把veth-ns1这个网卡按到了容器中，然后我们要把lxcbr0.1添加上网桥上
>
> brctl addif lxcbr0 lxcbr0.1
>
> 为容器增加一个路由规则，让容器可以访问外面的网络
>
> ip netns exec ns1     ip route add default via 192.168.10.1
>
> 在/etc/netns下创建network namespce名称为ns1的目录，
>
> 然后为这个namespace设置resolv.conf，这样，容器内就可以访问域名了
>
> mkdir -p /etc/netns/ns1
> echo "nameserver 8.8.8.8" > /etc/netns/ns1/resolv.conf

以下是NAT参考：

> 步骤
> 1	 创建名称为 ‘myspace’ 的 linux network namespace
>
> ip netns add myspace	                                               
> 2	 创建一个 veth 设备，一头为 veth1，另一头为 veth2
>
> ip link add veth1 type veth peer name veth2	            
> 3	将 veth2 加入 myspace 作为其一个 network interface
>
> ip link set veth2 netns myspace	               
> 4	配置 veth1 的 IP 地址
>
> ifconfig veth1 192.168.45.2 netmask 255.255.255.0 up	
> 5	配置 veth2 的 IP 地址，它和 veth1 需要在同一个网段上
>
> ip netns exec myspace ifconfig veth2 192.168.45.3 netmask 255.255.255.0 up	
> 6	  将 myspace 的默认路由设为 veth1 的 IP 地址
>
> ip netns exec myspace route add default gw 192.168.45.2	             
> 7	开启 linux kernel ip forwarding
>
> echo 1 > /proc/sys/net/ipv4/ip_forward   	
> 8	配置 SNAT，将从 myspace 发出的网络包的 soruce IP address 替换为 eth0 的 IP 地址
>
> iptables -t nat -A POSTROUTING -s 192.168.45.0/24 -o eth0 -j MASQUERADE	       
>
> 9	在默认 FORWARD 规则为 DROP 时显式地允许 veth1 和 eth0 之间的 forwarding
> iptables -t filter -A FORWARD -i eth0 -o veth1 -j ACCEPT
>
> iptables -t filter -A FORWARD -o eth0 -i veth1 -j ACCEPT
>