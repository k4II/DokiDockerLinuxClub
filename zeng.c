#define _GNU_SOURCE
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <sched.h>
#include <signal.h>
#include <unistd.h>
#include <sys/mount.h>
#include <stdlib.h>


#define STACK_SIZE (1024 * 1024)
 
static char child_stack[STACK_SIZE];
char* const child_args[] = {
  "/bin/bash",
  NULL
};

int checkpoint[2];
char c;

int child_main(void* arg)
{
 
  close(checkpoint[1]);

  read(checkpoint[0], &c, 1);
 
  printf(" Child -- %5d\n", getpid());

  sethostname("zengwei", 7);
  //挂载/proc
  system("mount -t proc proc /proc");
  //设置容器的IP:172.8.0.8
  system("ifconfig veth1 172.8.0.8");
  system("ip link set veth1 up");
  
  close(checkpoint[0]);


  execv(child_args[0], child_args);
  printf("Fail!\n");
  return 1;
}

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
 
int main()
{
  pipe(checkpoint);
  //获取容器外的UID/GID
  const int gid=getgid(), uid=getuid();

  printf(" Father -- %5d\n", getpid());
 
  int child_pid = clone(child_main, child_stack+STACK_SIZE,
      CLONE_NEWUTS | CLONE_NEWIPC | CLONE_NEWPID | CLONE_NEWNS |CLONE_NEWUSER |CLONE_NEWNET | SIGCHLD, NULL);
  //设置容器内UID/GID的映射关系
  set_uid_map(child_pid, 0, uid, 1);
  set_gid_map(child_pid, 0, gid, 1);
  
  char* cmd;
  //创建新的veth设备对，并将veth1放入容器
  asprintf(&cmd, "ip link set veth1 netns %d", child_pid);
  system("ip link add veth0 type veth peer name veth1");
  system(cmd);
  
  //创建虚拟网桥
  system("brctl addbr br-demo");
  system("brctl addif br-demo veth0");
  
  //设置宿主机IP：172.8.0.1
  system("sudo ifconfig br-demo 172.8.0.1");
  system("ip link set veth0 up");
  free(cmd); 
  
  //等待子进程处理
  close(checkpoint[1]);
  read(checkpoint[0], &c, 1);  
  
  //添加如下两条 iptables 规则后即可直接在本机地址上访问容器8088端口
  system("iptables -t nat -A PREROUTING -p tcp -m tcp --dport 8088 -j DNAT --to-destination 172.8.0.8:8000");
  system("iptables -t filter -A FORWARD -p tcp -m tcp --dport 8000 -j ACCEPT");
  close(checkpoint[1]);
 
  waitpid(child_pid, NULL, 0);

  return 0;
}
