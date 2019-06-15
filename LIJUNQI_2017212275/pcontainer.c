#define _GNU_SOURCE
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/mount.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/capability.h>
#include <sched.h>
#include <signal.h>
#include <unistd.h>


#define STACK_SIZE (1024 * 1024)
static char container_stack[STACK_SIZE];

int pipefd[2];

void set_map(pid_t pid, int uid, int gid) {
	//Map uid
	char uid_filename[256];
	sprintf(uid_filename, "/proc/%d/uid_map", pid);
	FILE* mapuid = fopen(uid_filename, "w");
	fprintf(mapuid, "0 %d 1",uid);
	fclose(mapuid);
	//Map gid
        char gid_filename[256];
	sprintf(gid_filename, "/proc/%d/gid_map", pid);
	FILE* mapgid = fopen(gid_filename, "w");
	fprintf(mapgid, "0 %d 1",gid);
	fclose(mapgid);
}


int pcontainer() /*Opeartions in the poor container*/
{
	//Wait until parent tells to continue
	char ch;
	close(pipefd[1]);
	read(pipefd[0],&ch,1);

	printf("Poor container with PID [%d]\n",getpid());
	mount("proc", "rootfs/proc","proc",0,NULL);

       	chdir("./rootfs");
	chroot("./");

	char* const args[] = {
	"/bin/sh",
	"-l",
	NULL
	};
	execv(args[0],args);
}

int main()
{
	printf("PARENT[%d]: Start the poor container!\n",getpid());
	//Use pipe to synchronize processes
	pipe(pipefd);

	int pid = clone(pcontainer,container_stack+STACK_SIZE, CLONE_NEWIPC | CLONE_NEWPID | CLONE_NEWNS | CLONE_NEWUSER | SIGCHLD, NULL);
	
	//Set the uid and gid map
	const int uid=getuid(), gid=getgid();
	set_map(pid ,uid ,gid);
	//Tell the children to continue
	close(pipefd[1]);


	waitpid(pid,NULL,0); /*Wait until all the opeartions are done */
	printf("PARENT: The poor container has been stopped!\n");
}
