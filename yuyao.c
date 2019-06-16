//  ./yuyao直接运行
//  抱歉只有基础要求,弱鸡哭泣.jpg 

#define _GNU_SOURCE
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <stdio.h>
#include <sched.h>
#include <signal.h>
#include <unistd.h>
#define STACK_SIZE (1024 * 1024)

static char container_stack[STACK_SIZE];
char* const container_args[] = {
	"/bin/bash",
	NULL
};



// 容器进程运行的程序主函数
int container_main(void *args)
{
	
	printf("Insider the A-New-bash...\nPID:%ld\n", (long)getpid()); // 查看子进程的PID	
	
	sethostname("(・ω・)",10);
	
	system("mount -t proc proc /proc");	
	system("echo \"\nChild--NET-LINK...:\"");
	system("ip link"); // check NET isolated
	system("id");	   // check uid and gid	
	
	execv(container_args[0], container_args); // 执行/bin/bash   return 1;
	
}




int main(int args, char *argv[])
{
	
	printf("程序开始\n");
	
	printf("Father[%d] - start A-New-bash!\n", getpid());
	printf("\nFather-NET--LINK...:\n");
	system("ip link"); // check NET
	printf("\n");
	system("ipcmk -Q"); // create queue ID, global = 0
	system("ipcs -q");	
	
	int container_pid = clone(container_main, container_stack + STACK_SIZE, SIGCHLD | CLONE_NEWUTS | CLONE_NEWIPC | CLONE_NEWPID | CLONE_NEWNS | CLONE_NEWNET, NULL);
	
	
	// 等待容器进程结束
	waitpid(container_pid, NULL, 0);
	return 0;
}
















