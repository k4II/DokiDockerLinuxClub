README.md

##### 使用方法

依次运行/源代码编译

```bash
sudo bash ./init.sh
sudo ./ns-demo	#gcc main.c -o ..
```

##### 备注：

`ubuntu18.04LTS`

默认使用 rootfs/bin/sh  添加bash支持需运行init.sh

`<sys/capability.h> --> install libcap-devel`

##### 完成功能： 

UTS,PID,MOUNT,USER,NET.

##### 测试：

容器内运行C编写的SimpleHTTPServer,容器目录`/SERVER/HTTPSVR`

默认宿主机访问
容器lxcveth1:8080(169.254.1.2:8080)

> 从源代码编译HTTPSVR
> ```
> gcc ./httpsvr.c --static -o HTTPSVR
> ```

##### 已知问题：

1. CLONE_NEWNET创建lxcveth0/1需要su.但与以普通用户运行容器相悖,造成parent root:container root的映射.权限过高
2. 尝试`sed ` 编辑`/proc/$$/uid,gid` 疑受`CAP_SETUID` 限制,改用C fileIO读写
