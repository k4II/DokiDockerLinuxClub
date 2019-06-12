# Test 尹德志
SRE
...Namespace--Linux with C

运用Namespace进行Linux的系统隔离。使用clone()或者unshare()，创建Namespace，在子进程中进行隔离。
我在这里用到了 

- CLONE_NEWNS		-- Mount Namespace 文件系统隔离
  使用挂载mount，将“/”中的目录与子进程进行共享
  或者用chroot，将子进程的目录设为“/”，但需要将原“/”下的/bin, /usr/bin等目录拷贝下来
  所以，运用挂载mount，会更方便  
  如 mount -t proc proc /proc 挂载/proc, 第二个proc 是mount的显示输出, 可以设为none
- CLONE_NEWPID	-- PID Namespace 进程隔离
  在clone()创建PID Nmaespace，进行父进程与子进程的隔离
  我们可以在创建子进程前后用getpid()查看当前进程号，检查是否成功隔离
  可以用 sudo ls -l /proc/$$/ns 命令查看当前进程
  可以用 ps -ef 命令检测进程
- CLONE_NEWNET	-- NET Namespace 网络隔离
  在clone()中创建NET Namespace，进行父子进程的网络隔离
  检查是否成功隔离，可以用 ip -link 命令查询当前网段，可以发现在子进程的网段变为了0，我们也可以手动设置当前网段
- CLONE_NEWIPC	-- IPC Namespace 通信隔离
  在clone()中创建IPC Namespace，进行父子进程的通信隔离
  首先，要在父进程中创建一个通信队列 ipcmk -Q , 它的全局ID = 0
  然后使用 ipcs -q 命令查询当前进程的消息队列
  可以看到在父进程中存在一个id = 0的队列，在最新的子进程中消息队列为空
- CLONE_NEWUTS	-- UTS Namespace 系统信息隔离
  uts可以查看系统的内核名称，版本等信息
  在clone()中创建UTS Namespase，进行父子进程的系统信息隔离
  可以在子进程中可以sethostname() 设置新的host name，可以看到我的子进程的的bash，与父进程的bash名称改变。(root权限)
- CLONE_NEWUSER	-- USER Namespace 用户隔离
  在clone()中创建USER Namespace，进行用户隔离(无root权限)
  使用 id 命令查看 uid 和 pid (nobody) 默认为最大值65534,因为不是真正的uid



应该是可以制作一个类似docker的隔离系统, 我的大概思路是创建一个rootfs文件夹当作container的"/",rootfs中同样含有bin/, usr/, lib/等文件.我是想把这些文件从"/'中copy下来, 然后一个个进行mount (如 mount -t  proc proc rootfs/proc), 然后chdir rootfs/   , chroot rootfs/. 这样一个新的根目录与主机隔离了.  我在操作的时候就是发现有些文件无法cp下来(sys/) .也没找到好的解决方法, 就没做了.
