# HomoDocker

​														——Sajo

##### 这么臭的容器有实装的必要吗？（半恼



## 1. 使用方法

运行环境：Linux 5.1.10-1.el7.elrepo.x86_64

### （1）下载源代码和rootfs

```markdown
[源码](https://pan.sajo.fun/s/gl7rwi90)
```

```markdown
[rootfs](https://pan.sajo.fun/s/6ttiyfjb)
```

### （2）解压到同一目录

```
unzip homodocker.zip
unzip homodocker-image.zip
```

### （3）编译

```
make
```

### （4） 运行

```
sudo ./homodocker
```



## 2.功能实现

### （1）Mount

![QQ20190616011142.png](https://tu.sajo.fun/images/2019/06/16/QQ20190616011142.png)

题目中给的rootfs没有bash 加入bash即可运行 此镜像为我本机拉取

![QQ20190616013455.png](https://tu.sajo.fun/images/2019/06/16/QQ20190616013455.png)



![QQ20190616013509.png](https://tu.sajo.fun/images/2019/06/16/QQ20190616013509.png)



![QQ20190616013705.png](https://tu.sajo.fun/images/2019/06/16/QQ20190616013705.png)



### （2）UTS

![QQ20190616011923.png](https://tu.sajo.fun/images/2019/06/16/QQ20190616011923.png)

手滑了

### （3）IPC

![QQ20190616013115.png](https://tu.sajo.fun/images/2019/06/16/QQ20190616013115.png)

### （4） PID

![QQ20190616013347.png](https://tu.sajo.fun/images/2019/06/16/QQ20190616013347.png)

### （5）User

![QQ20190616015001.png](https://tu.sajo.fun/images/2019/06/16/QQ20190616015001.png)

### （6）Network

此处使用了docker0网桥（手动配置的内容太多了

#### 1.查看docker0

```root@Sajo ~/114514# docker network inspect bridge
root@Sajo ~/114514# docker network inspect bridge
[
    {
        "Name": "bridge",
        "Id": "ab14a29a7b39bd4327eb2eee43df4a9ad32778aae34c51cf14751667122ff941",
        "Created": "2019-06-15T23:57:03.184570866+08:00",
        "Scope": "local",
        "Driver": "bridge",
        "EnableIPv6": false,
        "IPAM": {
            "Driver": "default",
            "Options": null,
            "Config": [
                {
                    "Subnet": "172.17.0.0/16",
                    "Gateway": "172.17.0.1"
                }
            ]
        },
        "Internal": false,
        "Attachable": false,
        "Containers": {},
        "Options": {
            "com.docker.network.bridge.default_bridge": "true",
            "com.docker.network.bridge.enable_icc": "true",
            "com.docker.network.bridge.enable_ip_masquerade": "true",
            "com.docker.network.bridge.host_binding_ipv4": "0.0.0.0",
            "com.docker.network.bridge.name": "docker0",
            "com.docker.network.driver.mtu": "1500"
        },
        "Labels": {}
    }
]
```

#### 2.修改main.cpp中的网桥ip和容器ip

![QQ20190616020350.png](https://tu.sajo.fun/images/2019/06/16/QQ20190616020350.png)

#### 3.开启宿主机转发

```vim /etc/sysctl.conf
vim /etc/sysctl.conf
net.ipv4.ip_forward = 1
sysctl -p
```

#### 4.DNS

```
vi /etc/resolv.conf
nameserver 8.8.8.8
```

宿主机防火墙

```iptables -t nat -A POSTROUTING -s 172.17.0.0/16 ! -o docker0 -j MASQUERADE
iptables -t nat -A POSTROUTING -s 172.17.0.0/16 ! -o docker0 -j MASQUERADE
```

#### 测试

![QQ20190616021207.png](https://tu.sajo.fun/images/2019/06/16/QQ20190616021207.png)



## 参考资料

<http://man7.org/linux/man-pages/man7/namespaces.7.html>

<https://angristan.xyz/setup-network-bridge-lxc-net/>

<https://github.com/lxc/lxc>

## 收获

网络部分直接用了lxc的轮子（过于偷工减料（摸鱼可能性微存

学习了先进的Namespace，Docker的本质是用LXC实现虚拟机的功能 来达到节约资源的目的

Control Group了解不多 加上网吧coding 就还没有完成 属实dd

搞了半天docker0 觉得自己计网学的一点都不扎实 需要不断巩固

啤酒烧烤伤身体 林檎先辈助健康（**そうだよ**

![IMG_20190616_023621.jpg](https://tu.sajo.fun/images/2019/06/16/IMG_20190616_023621.jpg)
![IMG_20190616_023625.jpg](https://tu.sajo.fun/images/2019/06/16/IMG_20190616_023625.jpg)
![IMG_20190616_023629.jpg](https://tu.sajo.fun/images/2019/06/16/IMG_20190616_023629.jpg)
![IMG_20190616_023634.jpg](https://tu.sajo.fun/images/2019/06/16/IMG_20190616_023634.jpg)