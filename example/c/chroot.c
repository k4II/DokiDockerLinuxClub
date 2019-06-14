#define _GNU_SOURCE
#include <sys/wait.h>
#include <sys/utsname.h>
#include <sched.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define errExit(msg)    do { perror(msg); exit(EXIT_FAILURE); \
				} while (0)
#define STACK_SIZE (1024 * 1024)    /* Stack size for cloned child */

static int              /* Start function for cloned child */
childFunc(void *arg)
{

	/* Keep the namespace open for a while, by sleeping.
	This allows some experimentation--for example, another
	process might join the namespace. */


	/*fork from man7.org and add 3 lines*/
	chdir("/tmp/newroot"); //change work dir
	chroot("/tmp/newroot");//change root dir
	system("/bin/sh"); //run shell

	return 0;           /* Child terminates now */
}



int main(int argc, char *argv[])
{
	char *stack;                    /* Start of stack buffer */
	char *stackTop;                 /* End of stack buffer */
	pid_t pid;
	struct utsname uts;



	/* Allocate stack for child */

	stack = malloc(STACK_SIZE);
	if (stack == NULL)
		errExit("malloc");
	stackTop = stack + STACK_SIZE;  /* Assume stack grows downward */

	/* Create child that has its own UTS namespace;
	child commences execution in childFunc() */

	pid = clone(childFunc, stackTop, CLONE_NEWUTS | SIGCHLD, argv[1]);
	if (pid == -1)
		errExit("clone");
	printf("clone() returned %ld\n", (long) pid);

	/* Parent falls through to here */

	sleep(1);           /* Give child time to change its hostname */

	/* Display hostname in parent's UTS namespace. This will be
	different from hostname in child's UTS namespace. */

	if (uname(&uts) == -1)
		errExit("uname");
	// printf("uts.nodename in parent: %s\n", uts.nodename);

	if (waitpid(pid, NULL, 0) == -1)    /* Wait for child */
		errExit("waitpid");
	//printf("child has terminated\n");

	exit(EXIT_SUCCESS);
}
