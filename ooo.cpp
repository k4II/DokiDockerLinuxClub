//
// Created by bigjj on 19-6-15.
//

#include <sys/stat.h>
#include <cstdio>
#include <unistd.h>
#include <sys/mount.h>
#include <signal.h>
#include <sched.h>
#include <wait.h>
#include <cstdlib>
#include "doooocker.h"
void mot(char *, char *, char *);
void set_user_map(pid_t, int ,bool);
static char stack[STACK_SIZE];

int pipe_fd[2];

static char * toMount[][3] = {
        {"proc", "proc", "proc"},
        //{"sysfs", "sys", "sysfs"},
        //{"none", "tmp", "tmpfs"},
        //{"udev", "dev", "devtmpfs"},
        //{"tmpfs", "run", "tmpfs"}
};

int copy_file() {
    if (mkdir("/tmp/r", S_IRWXU) != 0) {
        perror("mkdir");
        return -1;
    }
    return 0;
}

void in_net() {
    system("ip link set in_veth up");
    system("ip link set lo up");
    system("ip addr add 193.168.10.11/24 dev in_veth");
}

int fn(void * args) {
    //copy_file();
    sethostname("small jj", 10);
    if (chdir("/tmp/r") != 0 )
        perror("cd");
    for (auto & i : toMount) {
        mot(i[0], i[1], i[2]);
    }
    if (chroot("/tmp/r/") != 0) {
        perror("chroot");
        return 0;
    }
    char bf;
    close(pipe_fd[1]);
    read(pipe_fd[0], &bf, 1);
    in_net();
    system("/bin/zsh");
    perror("exe");
    return 1;
}

void mot(char * source, char * target, char * type) {
    if (mount(source, target, type, 0, nullptr) != 0) {
        perror(target);
    }
}

void set_user_map(pid_t pid, int id, bool is_u) {
    char file_name[256];
    sprintf(file_name, "/proc/%d/%cid_map", pid, is_u ? 'u' : 'g');
    FILE * mapfd = fopen(file_name, "w");
    if (NULL == mapfd) {
        perror("create map file");
        return;
    }
    fprintf(mapfd, "%d %d %d", 0, id, 1);
    fclose(mapfd);
}

void out_net(int pid) {
    char bf[256];
    system("ip link add out_veth type veth peer name in_veth");
    sprintf(&bf, "ip link set in_veth netns %d", pid);
    system(bf);
    system("ip link set out_veth up");
}

int start() {
    const int gid = getgid(), uid = getuid();
    pipe(pipe_fd);
    int pid = clone(fn, stack + STACK_SIZE,
                    CLONE_NEWUTS
                    | SIGCHLD
                    | CLONE_NEWIPC
                    | CLONE_NEWPID
                    | CLONE_NEWNS
                    | CLONE_NEWNET
                    | CLONE_NEWUSER, nullptr);
    out_net(pid);
    set_user_map(pid, uid, true);
    set_user_map(pid, gid, false);
    close(pipe_fd[1]);
    return pid;
}
