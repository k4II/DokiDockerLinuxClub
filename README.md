###         红岩期末考核

完成了进程隔离，IPC隔离，用户隔离，网络隔离，端口映射。

编译后用"sudo ./zeng"运行
可在容器内用"python -m SimpleHTTPServer"运行简单的http服务，在容器外访问宿主机的8088端口即可访问到容器的8000端口"curl -I 127.0.0.1:8088"

体会：文件系统隔离杀我...想过是用kaii给的rootfs做个类似docker的效果，可除了"mount -t proc proc /proc"可以成功其他的都mount不动....更换目录也失败..

收获：做端口映射的时候了解了一点linux虚拟网卡的知识，还有namespace牛逼┗|｀O′|┛ 嗷~~











