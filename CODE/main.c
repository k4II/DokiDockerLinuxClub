#define _GNU_SOURCE
#include <sys/wait.h>
#include <sys/utsname.h>
#include <sys/mount.h>
#include <sys/capability.h>
#include <sched.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define errExit(msg)    do { perror(msg); exit(EXIT_FAILURE); \
				} while (0)
#define STACK_SIZE (1024 * 1024)
//PUBLIC
char* const container_args[]={
    "/bin/sh",
    NULL
};
int pipefd[2];//checkpoint


////Packaged Func
//MOUNT (child)
void mkdir_mount()
{   if(access("/proc",F_OK)==-1)
        system("mkdir /proc");
    if(mount("proc","/proc","proc",MS_NOEXEC|MS_NODEV|MS_NOSUID,NULL)==-1)
        errExit("mount");
}
//USER (parent)
void set_map(char* file,int outside_id) {
    FILE* mapfd = fopen(file, "w");
    if (NULL == mapfd) {
        perror("open file error");
        return;
    }
    fprintf(mapfd, "0 %d 1",outside_id);
    fclose(mapfd);
}
void set_uid_map(pid_t pid,int outside_id) {
    char uid_map[256];
    sprintf(uid_map, "/proc/%d/uid_map", pid);
    set_map(uid_map,outside_id);
}
void set_gid_map(pid_t pid,int outside_id) {
    char gid_map[256];
    sprintf(gid_map, "/proc/%d/gid_map", pid);
    set_map(gid_map,outside_id);
}//USER

//NET (both)
void set_parent_net(int child_pid)
{
    char cmd[256];
    //veth初始化：创建一个veth对
    system("ip link add lxcveth0 type veth peer name lxcveth1");
	sprintf(cmd,"ip link set lxcveth1 netns %d",child_pid);
    system(cmd);
    system("ip link set lxcveth0 up");
    system("ip addr add 169.254.1.1/30 dev lxcveth0");
}
void set_child_net()
{
    char *c;
    read(pipefd[0],&c,1);

    system("ip link set lo up");
    system("ip link set lxcveth1 up");
    system("ip addr add 169.254.1.2/30 dev lxcveth1");
}//NET



static int              /* Start function for cloned child */
child_main(void *arg)
{
    /* 等待父进程通知后再往下执行（进程间的同步） */
    char c;
    close(pipefd[1]);
    read(pipefd[0], &c, 1);

    //info
	printf("Container [%5d] - inside the container!\n", getpid());
	printf("Container: eUID = %ld;  eGID = %ld, UID=%ld, GID=%ld\n",
            (long) geteuid(), (long) getegid(), (long) getuid(), (long) getgid());



    set_child_net();
	if(chdir("/tmp/newroot")==-1) //change work dir
        errExit("chdir");
	if(chroot("/tmp/newroot")==-1)//change root dir
        errExit("chroot");
    /*finish remained namespaces.
    UTS,MOUNT(re),NET.
    */
    sethostname("container",10);//uts
    mkdir_mount();//mount





	execv(container_args[0], container_args);
    //run shell.

	return 1;   //something's wrong.
}


int main(int argc, char *argv[])
{
	char *stack;
	char *stackTop;
	stack = malloc(STACK_SIZE); //stack for child
	if (stack == NULL)
		errExit("malloc");
	stackTop = stack + STACK_SIZE;  /* Assume stack grows downward */

    pid_t pid;  //child pid in outerspace.
	struct utsname uts;
    const int gid=getgid(), uid=getuid();


    //info
    printf("Parent [%5d] - start a container!\n", getpid());
    printf("Parent: eUID = %ld;  eGID = %ld, UID=%ld, GID=%ld\n",
            (long) geteuid(), (long) getegid(), (long) getuid(), (long) getgid());


    pipe(pipefd);
	/* Create child that has its own
	UTS namespace
	PID namespace
	USER namespace
	MOUNT namespace
	NET namespace*/
	pid = clone(child_main, stackTop,  CLONE_NEWUTS| CLONE_NEWPID |
	 CLONE_NEWUSER | CLONE_NEWNS  | CLONE_NEWNET | CLONE_NEWIPC |SIGCHLD, argv[1]);

    //USER
    set_gid_map(pid,gid);
    set_uid_map(pid,uid);
    printf("Parent [%5d] - user/group mapping done!\n", getpid());
    //NET
    set_parent_net(pid);

    close(pipefd[1]);//notice child




	if (pid == -1)
		errExit("clone");
	printf("clone() returned %ld\n", (long) pid);


	sleep(1);

	if (waitpid(pid, NULL, 0) == -1)
		errExit("waitpid");
	printf("Parent:child stopped\n");

	exit(EXIT_SUCCESS);
}
