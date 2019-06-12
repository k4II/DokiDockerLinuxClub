// use ( sudo ./my ) to run
#define _GNU_SOURCE
#include <sys/wait.h>
#include <sys/utsname.h>
#include <sys/mount.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#define STACK_SIZE (1024 * 1024)

static char childStack[STACK_SIZE];
char *const A_New_bash[] =
    {
        "/bin/bash",
        NULL};
static int childFunc()
{
    printf("Insider the A-New-bash...\nPID:%ld\n", (long)getpid()); // 查看子进程的PID，其输出子进程的 pid 为 1
    sethostname("A-New-bash", 11);                                  // set host name
    unshare(CLONE_NEWNET);                                          // NET isolated | use in clone()

    if (mount("proc", "/proc", "proc", 0, NULL) != 0)
    {
        perror("proc");
    }
    if (mount("sysfs", "/sys", "sysfs", 0, NULL) != 0)
    {
        perror("sys");
    }
    if (mount("none", "/tmp", "tmpfs", 0, NULL) != 0)
    {
        perror("tmp");
    }
    if (mount("udev", "/dev", "devtmpfs", 0, NULL) != 0)
    {
        perror("dev");
    }
    if (mount("devpts", "/dev/pts", "devpts", 0, NULL) != 0)
    {
        perror("dev/pts");
    }
    if (mount("shm", "/dev/shm", "tmpfs", 0, NULL) != 0)
    {
        perror("dev/shm");
    }
    if (mount("tmpfs", "/run", "tmpfs", 0, NULL) != 0)
    {
        perror("run");
    }
    // chdir("/rootfs");  //change work dir
    // chroot("/rootfs"); //change root dir
    // system("sudo ls -l /proc/$$/ns");	// check PID isolated
    system("echo \"\nChild--NET-LINK...:\"");
    system("ip link"); // check NET isolated
    system("id");      // check uid and gid
    execv(A_New_bash[0], A_New_bash);
    // system("ipcs -q"); // check ipc isolated
    return 1;
}
int main()
{
    printf("Father[%d] - start A-New-bash!\n", getpid());
    printf("\nFather-NET--LINK...:\n");
    system("ip link"); // check NET
    printf("\n");
    system("ipcmk -Q"); // create queue ID, global = 0
    system("ipcs -q");
    pid_t child_pid = clone(childFunc, childStack + STACK_SIZE,
                            CLONE_NEWUSER | CLONE_NEWNS | CLONE_NEWUTS | CLONE_NEWPID | CLONE_NEWIPC | SIGCHLD, NULL); // 传尾指针，因为栈是反着的
    // use CLONE_NEWNS, CLONE_NEWIPC, CLONE_NEWNET, CLONE_NEWUTS, CLONE_PID : __Namespace__
    if (child_pid == -1)
    {
        perror("clone");
        exit(EXIT_FAILURE);
    }
    printf("clone()=%ld\n", (long)child_pid);
    sleep(1);

    struct utsname uts; // 获取当前内核名称和其它信息, return 0 is successful, -1 is failure
    if (uname(&uts) == -1)
    {
        perror("uname");
        exit(EXIT_FAILURE);
    }
    printf("uts.nodename in parent: %s\n", uts.nodename);
    if (waitpid(child_pid, NULL, 0) == -1)
    {
        perror("waitpid");
        exit(EXIT_FAILURE);
    }
    printf("Child has terminated\n");
    return 0;
}