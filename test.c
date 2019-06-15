#define _GNU_SOURCE
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/mount.h>
#include <sys/capability.h>
#include <stdio.h>
#include <sched.h>
#include <signal.h>
#include <unistd.h> 
#define STACK_SIZE (1024 * 1024)//定义一个给 clone 用的栈，栈大小1M
 
static char container_stack[STACK_SIZE];
char* const container_args[] = {
    "/bin/bash",//直接执行一个shell，以便我们观察这个进程空间里的资源是否被隔离了
    NULL
};
int pipefd[2];
char ch;

void set_map(char* file, int inside_id, int outside_id, int len) {
    FILE* mapfd = fopen(file, "w");
    if (NULL == mapfd) {
        perror("open file error");
        return;
    }
    fprintf(mapfd, "%d %d %d", inside_id, outside_id, len);//fprintf,是把格式化字符串输出到指定文件中,所以参数比printf多了个文件指针File * ,那是目标文件的文件描述符(文件流指针)
    fclose(mapfd);
}

void set_uid_map(pid_t pid, int inside_id, int outside_id, int len) {
    char file[256];
    sprintf(file, "/proc/%d/uid_map", pid);//sprintf,是把格式化字符串输出到指定字符串,所以参数比printf多了个char * ,那就是目标字符串的地址.
    set_map(file, inside_id, outside_id, len);
}

void set_gid_map(pid_t pid, int inside_id, int outside_id, int len) {
    char file[256];
    sprintf(file, "/proc/%d/gid_map", pid);
    set_map(file, inside_id, outside_id, len);
}
 
int container_main(void* arg)
{
    printf("Container [%5d] - inside the container!\n", getpid());
    printf("Container: eUID = %ld;  eGID = %ld, UID=%ld, GID=%ld\n",
            (long) geteuid(), (long) getegid(), (long) getuid(), (long) getgid());
 
    /* 等待父进程通知后再往下执行（进程间的同步） */
    close(pipefd[1]);
    read(pipefd[0], &ch, 1);
 
    printf("Container [%5d] - setup hostname!\n", getpid());
    
    //set hostname
    sethostname("container",10);
 
    //remount "/proc" to make sure the "top" and "ps" show container's information

   if (mount("proc", "/proc", "proc",0, NULL) !=0 ) {
        perror("proc");
    }
   if (mount("sys","/sys","sysfs",0,NULL)!=0)
    {
	    perror("sys");
    }
   if (mount("dev","/dev","tmpfs",0,NULL)!=0)
    {
            perror("dev");
    }
   if (mount("run","/run","tmpfs",0,NULL)!=0)
    {
            perror("run");
    }
   if (mount("tmp","/tmp","tmpfs",0,NULL)!=0)
    {
            perror("tmp");
    }
   if (system("mount --bind rootfs ./mnt")!=0)
   {
	   perror("mount_rootfs");
   }

   /* chroot 隔离目录 */
   /*if ( chdir("./rootfs") != 0 || chroot("./") != 0 ){
        perror("chdir/chroot");
    }*/
    system("ip link set dev lo up");//激活namespace中的loopback，即127.0.0.1 
    system("ifconfig veth1 192.168.186.127");//为容器中的网卡分配一个IP地址
    system("ip link set veth1 up");//激活
    system("ip route add default via 192.168.186.12");//为容器增加一个路由规则，让容器可以访问外面的网络

    close(pipefd[0]);
    execv(container_args[0], container_args);
    perror("exec");
    printf("Something's wrong!\n");
    return 1;
}
 
int main()
{  
    pipe(pipefd);//gain the uid/gid
    const int gid=getgid(), uid=getuid();
 
    printf("Parent: eUID = %ld;  eGID = %ld, UID=%ld, GID=%ld\n",
            (long) geteuid(), (long) getegid(), (long) getuid(), (long) getgid());
 
    
    printf("Parent [%5d] - start a container!\n", getpid());
    int container_pid = clone(container_main, container_stack+STACK_SIZE, 
            CLONE_NEWUTS | CLONE_NEWIPC | CLONE_NEWPID | CLONE_NEWNS | CLONE_NEWUSER | CLONE_NEWNET | SIGCHLD, NULL);//调用clone函数，其中传出一个函数，还有一个栈空间的（为什么传尾指针，因为栈是反着的）
    printf("Parent [%5d] - Container [%5d]!\n", getpid(), container_pid);
 
    //To map the uid/gid, 
    //   we need edit the /proc/PID/uid_map (or /proc/PID/gid_map) in parent
    //The file format is
    //   ID-inside-ns   ID-outside-ns   length
    //if no mapping, 
    //   the uid will be taken from /proc/sys/kernel/overflowuid
    //   the gid will be taken from /proc/sys/kernel/overflowgid
    set_uid_map(container_pid, 0, uid, 1);
    set_gid_map(container_pid, 0, gid, 1);
 
    printf("Parent [%5d] - user/group mapping done!\n", getpid());
  //首先，我们先增加一个网桥bridge0
    system("brctl addbr bridge0");
    system("brctl stp bridge0 off");
    system("ifconfig bridge0 192.168.186.12 ");//为网桥设置IP地址*/

    char *exec1;
    asprintf(&exec1,"ip link set veth1 netns %d",container_pid);//把 veth2 按到namespace中，这样容器中就会有一个新的网卡了
    system("ip link add veth0 type veth peer name veth1");//增加虚拟网卡,注意其中的veth类型，其中一个网卡要按进容器中
    system(exec1);
    system("brctl addif bridge0 veth0");//上面我们把veth1这个网卡按到了容器中，然后我们要把veth0添加上网桥上
    system("ip link set veth0 up");
    free(exec1);
    char *exec2;
    asprintf(&exec2,"mkdir -p /etc/netns/%d && echo \"nameserver 8.8.8.8\" > /etc/netns/%d/resolv.conf",container_pid,container_pid);//在/etc/netns下创建network namespce目录(名称为namespace的名称)，然后为这个namespace设置resolv.conf，这样，容器内就可以访问域名了
    system(exec2);

   system("iptables -t nat -A PREROUTING -p tcp -m tcp --dport 1234 -j DNAT --to-destination 192.168.186.127:2345");
   system("iptables -t filter -A FORWARD -p tcp -m tcp --dport 2345 -j ACCEPT");

    /* 通知子进程 */
    close(pipefd[1]);
  
    waitpid(container_pid, NULL, 0);//等待子进程结束
    printf("Parent - container stopped!\n");
    return 0;
}
