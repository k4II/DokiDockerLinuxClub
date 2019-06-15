#define _GNU_SOURCE
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>
#include <sched.h>
#define STACK_SIZE (1024*1024)
static char container_stack[STACK_SIZE]; /*设置子进程的堆栈空间大小*/

/*user 的映射*/
void set_map(char* file, int inside_id, int outside_id, int len) {
    FILE* mapfd = fopen(file, "w");
    if (NULL == mapfd) {
        perror("open file error");
        return;
    }
    fprintf(mapfd, "%d %d %d", inside_id, outside_id, len);
    fclose(mapfd);
}

void set_uid_map(int pid, int inside_id, int outside_id, int len) {
    char file[256];
    sprintf(file, "/proc/%d/uid_map", pid);
    set_map(file, inside_id, outside_id, len);
}

void set_gid_map(int pid, int inside_id, int outside_id, int len) {
    char file[256];
    sprintf(file, "/proc/%d/gid_map", pid);
    set_map(file, inside_id, outside_id, len);
}

/*net的隔离*/
void parent_net(int child_pid)
{
        char cmd[255];

        /*创建两张虚拟网卡*/
        system(" ip link add veth_p type veth peer name veth_c");

        /*将veth_c网卡放在子进程中，保存在字符串 cmd 中*/
        sprintf(&cmd,"ip link set veth_c netns %d",child_pid);
        system(cmd);

        /*因为默认是关闭的，所以我们需要手动打开，并且给父进程分配IP*/
        system("ip link set veth_p up");
        system("ip addr add 192.168.75.4/24 dev veth_p");
}

void child_net()
{       /*打开并分配IP*/
        system("ip link set veth_c up");
        system("ip addr add 192.168.75.3/24 dev veth_c");

        /*通过ping进行测试，ping的了就OK*/
        printf("现在开始ping 192.168.75.4（父进程）\n");
        system("ping -c 2 192.168.75.4");
        printf("-----------------------------------\n");

}
/*net端口映射*/
int port()
{
        system("iptables -t nat -A PREROUTING -d 192.168.75.4 -p tcp --dport 80 -j DNAT --to-destination 192.168.75.3:8000");
        system("iptables -t nat -A POSTROUTING -d 192.168.75.3 -p tcp --dport 8000 -j SNAT 192.168.75.4");
        return 1;
}

/*限制容器资源,要在父进程中*/
void control_size(int child_pid)
{
        char cmd[255];
        system("mkdir /sys/fs/cgroup/memory/a_test");
        system("echo 50*1000000 > /sys/fs/cgroup/memory/a_test/memory.limit_in_bytes");
        sprintf(&cmd,"echo %d > /sys/fs/cgroup/memory/mygroup/tasks",child_pid);
        system(cmd);

}
/*用来打印每个 ns 的相关信息*/
void printf_show(void)
{
        system("readlink /proc/$$/ns/pid");
        system("readlink /proc/$$/ns/mnt");
        system("readlink /proc/$$/ns/net");
        system("readlink /proc/$$/ns/user");
        system("readlink /proc/$$/ns/ipc");
        system("readlink /proc/$$/ns/uts");
        system("ip link");
        printf("/n");
}

/*子进程启动的函数*/
int child_func(void *arc)
{
        /*用来打印提示和相关信息*/
        system("mount -t proc proc /proc");
        printf("-----------------------------------\n");
        printf("welcome to the new space!\n");

        /*设置下主机名*/
        sethostname("kinco", 5);
        printf("now the host name is kinco!\n");

        /*输出新的ns 和 net信息*/
        child_net();
        printf("the new pid : %d \n", getpid());
        printf_show();

        /*隔离文件系统，并启动shell*/
        chdir("/tmp/rootfs");
        chroot("/tmp/rootfs");
        system("/bin/bash");
        return 0;
}

int main(void)
{       /*打印原来进程中的ns和net信息*/
        printf("the old ns: \n");
        printf_show();
        const int gid = getgid(),uid = getuid();

        /*一个子进程，满足了五个ns隔离*/
        int clone_pid = clone(child_func,
                              container_stack + STACK_SIZE,
                              CLONE_NEWPID | CLONE_NEWNET | CLONE_NEWUTS | CLONE_NEWIPC | CLONE_NEWNS | CLONE_USER | SIGCHLD ,
                              NULL);
        if (clone_pid < 0)
            {
                printf("clone error!\n");
                exit(EXIT_FAILURE);
            }
         /*net*/
         parent_net(clone_pid);

        /*user*/
        set_uid_map(clone_pid, 0, uid, 1);
        set_gid_map(clone_pid, 0, gid, 1);

        sleep(2);

        if(waitpid(clone_pid, NULL, 0) == -1)
            {
                printf("waitpid error!\n");
            exit(EXIT_FAILURE);
            }

            /*重新挂一下*/
        system("mount -t proc proc /proc");

            /*提示子进程运行结束*/
        printf("back to the parent.");

        return 0;
}
