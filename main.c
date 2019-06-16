#define _GNU_SOURCE
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/mount.h>
#include <stdio.h>
#include <stdlib.h>
#include <sched.h>  //provide some function
#include <unistd.h>
#include <signal.h>

#define STACK_SIZE (1024*1024)  //define a 1mb stack to clone function

/*int clone(int (*fn)(void *), void *child_stack,int flags, void *arg)*/ 
int checkpoint[2];
static char child_stack[STACK_SIZE];
void mountdir();
void parent_netset();
void child_netnamespace();
int child_main(void *arg);
void set_map(char* file, int inside_id, int outside_id, int len);
void set_uid_map(pid_t pid, int inside_id, int outside_id, int len);
void set_gid_map(pid_t pid, int inside_id, int outside_id, int len);
char * const child_arg[]={
    "/bin/bash",
    NULL
};

int main()
{
    int child_pid;
    const int gid=getgid(), uid=getuid();
    char *child_stack_top;
    printf("Parent: eUID = %ld;  eGID = %ld, UID=%ld, GID=%ld\n",
            (long) geteuid(), (long) getegid(), (long) getuid(), (long) getgid());
    pipe(checkpoint);
    printf("Parent[%5d]: start a clone process!\n",getpid());

    //启用UTS namespace
    //启用IPC namespace
    //启用PID namespace
    //启用Mount namespace
    //启用NET namespace
    child_stack_top = child_stack + STACK_SIZE;
    child_pid = clone(child_main,child_stack_top,
        CLONE_NEWUSER|CLONE_NEWUTS|CLONE_NEWIPC|CLONE_NEWPID|CLONE_NEWNS|CLONE_NEWNET|SIGCHLD ,NULL);
    printf("Parent [%5d] - Child [%5d]!\n", getpid(), child_pid);

    //设置uid/gid
    set_uid_map(child_pid, 0, uid, 1);
    set_gid_map(child_pid, 0, gid, 1);
    printf("Parent [%5d] - user/group mapping done!\n", getpid());

    ////veth初始化：创建一个veth对（抽离成了函数）
    parent_netset(child_pid);

    //向检查点传输父进程网络设置完成的信号
    close(checkpoint[1]);

    //等待子进程结束
    waitpid(child_pid,NULL,0);
    printf("Parent: a cloned process has been stopped!\n");
}

// int (*fn)(void *)
int child_main(void *arg)
{
    int flag;
    char *path;
    char defaulthostname[30]={"child"};
    close(checkpoint[1]);

    //查看子进程PID
    printf("Child [%5d]: Inside the child process\n",getpid());
    printf("Child: eUID = %ld;  eGID = %ld, UID=%ld, GID=%ld\n",
            (long) geteuid(), (long) getegid(), (long) getuid(), (long) getgid());

    //设置hostname（抽离成了函数）
    printf("Child [%5d] - setup hostname!\n", getpid());
    sethostname(defaulthostname,sizeof(defaulthostname));

    //获取当前文件所在绝对路径
    if((path = getcwd(NULL,0))==NULL)
    {
        perror("Getcwd error!");
        exit(1);
    }
    else
    {
        //更改chroot
        chdir(path);
        chroot(".");
        free(path);
    }
    
    //重新挂载文件系统（抽离成了函数）
    mountdir();

    //设置网络隔离（抽离成了函数）
    child_netnamespace();

    //execv only return -1 when it was wrong
    flag = execv(child_arg[0],child_arg);
    if(flag == -1)
    {
        printf("Something is Wrong!\n");
    }
    return 1;
}

void mountdir()
{
    /* int mount(const char *source, const char *target,
                const char *filesystemtype, unsigned long mountflags, const void *data);*/
    mount("proc", "/proc", "proc", MS_NOSUID|MS_NODEV|MS_NOEXEC, NULL);
    mount("sys", "/sys", "sysfs", MS_NOSUID|MS_NODEV|MS_NOEXEC, NULL);
    mount("tmp", "/tmp", "tmpfs", MS_NOSUID|MS_NODEV, NULL);
    mount("dev", "/dev", "tmpfs", MS_NOSUID|MS_NOEXEC, NULL);
    mount("run", "/run", "tmpfs", MS_NOSUID|MS_NODEV,NULL);
}

void child_netnamespace()
{
    char *c;

    //等待父进程完成网络设置
    read(checkpoint[0],&c,1);

    //设置网络
    system("ip link set lo up");
    system("ip link set veth1 up");
    system("ip addr add 169.254.1.2/30 dev veth1");
}

void parent_netset(int child_pid)
{
    char *cmd;

    //veth初始化：创建一个veth对
    asprintf(&cmd,"ip link set veth1 netns %d",child_pid); //获取namespace的pid拼接成cmd命令
    system("ip link add veth0 type veth peer name veth1");
    system(cmd);
    system("ip link set veth0 up");
    system("ip addr add 169.254.1.1/30 dev veth0");
    free(cmd);
}

void set_map(char* file, int inside_id, int outside_id, int len) 
{
    FILE* mapfd = fopen(file, "w");
    if (NULL == mapfd) {
        perror("open file error");
        return;
    }
    fprintf(mapfd, "%d %d %d", inside_id, outside_id, len);
    fclose(mapfd);
}
 
void set_uid_map(pid_t pid, int inside_id, int outside_id, int len) 
{
    char file[256];
    sprintf(file, "/proc/%d/uid_map", pid);
    set_map(file, inside_id, outside_id, len);
}
 
void set_gid_map(pid_t pid, int inside_id, int outside_id, int len) 
{
    char file[256];
    sprintf(file, "/proc/%d/gid_map", pid);
    set_map(file, inside_id, outside_id, len);
}
