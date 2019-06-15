#define _GNU_SOURCE
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/mount.h>
#include <sys/capability.h>
#include <stdio.h>
#include <sched.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
 
#define STACK_SIZE (1024 * 1024)
 
static char container_stack[STACK_SIZE];
char* const container_args[] = {
    "/bin/bash",
    NULL
};

int pipefd[2];

void set_map(char* file, int inside_id, int outside_id, int len) {
    FILE* mapfd = fopen(file, "w");
    if (NULL == mapfd) {
        perror("open file error");
        return;
    }
    fprintf(mapfd, "%d %d %d", inside_id, outside_id, len);
    fclose(mapfd);
}
 
void set_uid_map(pid_t pid, int inside_id, int outside_id, int len) {
    char file[256];
    sprintf(file, "/proc/%d/uid_map", pid);
    set_map(file, inside_id, outside_id, len);
}
 
void set_gid_map(pid_t pid, int inside_id, int outside_id, int len) {
    char file[256];
    sprintf(file, "/proc/%d/gid_map", pid);
    set_map(file, inside_id, outside_id, len);
}

void parent_net()
{
    //父进程网络
    system("echo 1 > /proc/sys/net/ipv4/ip_forward");
    system("ip netns add ns1");
    system("ip link add veth0 type veth peer name veth1");
    system("ip link set veth1 netns ns1");
    system("brctl addbr br-demo");
    system("brctl addif br-demo veth0");
    system("ifconfig br-demo 172.8.0.1");
    system("ip link set veth0 up");
    system("iptables -t filter -A FORWARD -i br-demo ! -o br-demo -j ACCEPT");
    system("iptables -t filter -A FORWARD -i br-demo -o br-demo -j ACCEPT");
    system("iptables -t filter -A FORWARD -o br-demo -m conntrack --ctstate RELATED,ESTABLISHED -j ACCEPT");
    system("iptables -t nat -A POSTROUTING -s 172.8.0.0/16 ! -o br-demo -j MASQUERADE");

}

void child_net()
{
    //子进程网络
    char *ch;
    read(pipefd[0],&ch,1);
    system("ip link set lo up");
    system("ip link set veth1 up");
    system("ip addr add 172.8.0.8 dev veth1");
}

int container_main(void* arg)
{
    char ch;
    printf("Container [%5d] - inside the container!\n", getpid());
    printf("Container: eUID = %ld;  eGID = %ld, UID=%ld, GID=%ld\n",
            (long) geteuid(), (long) getegid(), (long) getuid(), (long) getgid());
    close(pipefd[1]);
    read(pipefd[0],&ch,1);
    printf("Container [%4d] - sethost\n",getpid());
    sethostname("container",10);
    //更改工作目录及根目录
    chdir("/rootfs");
    chroot("/rootfs");
    //文件挂载
    if (mount("proc", "/proc", "proc", 0, NULL) !=0 ) {
        perror("proc");
    }
    if (mount("sys", "/sys", "sysfs", 0, NULL) !=0) {
        perror("sys");
    }
    if (mount("tmp", "/tmp", "tmpfs", 0, NULL) !=0) {
        perror("tmp");
    }
    if (mount("dev", "/dev", "tmpfs", 0, NULL) !=0) {
        perror("dev");
    }
    child_net();
    execv(container_args[0], container_args);
    perror("exec");
    printf("Something's wrong!\n");
    return 1;
}
 
int main()
{
    const int gid=getgid(), uid=getuid();
    //用于查看用户id
    printf("Parent: eUID = %ld;  eGID = %ld, UID=%ld, GID=%ld\n",
            (long) geteuid(), (long) getegid(), (long) getuid(), (long) getgid());

    pipe(pipefd);

    printf("Parent [%5d] - start a container!\n", getpid());
    //clone是个强大的函数，可以实现6种资源隔离
    int container_pid = clone(container_main, container_stack+STACK_SIZE,
            CLONE_NEWUTS | CLONE_NEWPID | CLONDE_NEWNS | CLONE_NEWNS | CLONE_NEWNET | CLONE_NEWUSER | SIGCHLD,NULL);

    printf("Parent [%5d] - Container [%5d]!\n", getpid(), container_pid);
    //id映射，抽离成函数
    set_uid_map(container_pid, 0, uid, 1);
    set_gid_map(container_pid, 0, gid, 1);

    printf("Parent [%5d] - user/group mapping done!\n", getpid());

    parent_net();

    close(pipefd[1]);

    waitpid(container_pid, NULL, 0);
    printf("Parent - container stopped!\n");
    return 0;
}
