[TOC]



# 记一次挣扎摸鱼

​		啊不擅长整理，感想和思路穿插着说吧……

## 环境依赖

- 个人测试为ubuntu18.04系统
- 纯净ubuntu应该是缺少gcc和libcap的,需要通过apt-get  build-dep  gcc和apt install libcap-dev命令安装
- 需要root权限

## 使用说明

1. 下载do.c主程序(别问为什么是do)和[rootfs.tar](https://pan.baidu.com/s/1D_ePoZqQbVOjUswsJC7vxQ)(fpy9)
2. 先解压还是先移动这种鸡蛋问题就不纠结了，总之要把压缩包里的rootfs文件夹放到根目录下也就是"/"目录(视情况提升权限)
3. sudo gcc do.c -o do此命令为编译并生成可执行文件do
4. ./do 运行程序

## 友链

因为看的实在太多数不胜数，挑一些收获颇丰的。

[试试Linux下的ip命令，ifconfig已经过时了](https://linux.cn/article-3144-1.html)

[DOCKER基础技术：LINUX NAMESPACE（下）](https://coolshell.cn/articles/17029.html)

[DOCKER基础技术：LINUX NAMESPACE（上）](https://coolshell.cn/articles/17010.html)

[Docker 核心技术与实现原理](https://draveness.me/docker)

[自己动手实现 Docker bridge network](https://hiberabyss.github.io/2018/02/02/docker-bridge-network-practice/)

[通过iptables实现端口转发和内网共享上网](http://xstarcd.github.io/wiki/Linux/iptables_forward_internetshare.html)

## 感想和猜测

- 我就沿着我的代码说一下感想。。。。这次写考核是一个十分漫长和焦灼的过程。
- 学到了很多东西，比如对容器，镜像和文件系统有了全新的认识。但许多疑问的地方仍不全得以解惑，仅发表一下自己的猜测和看法。
- 遇到的第一个小坑是在实现uts隔离时，虽然应该是ubuntu系统的机制，还是分享一下：在普通用户下sudo运行程序似乎有次数限制(?)，于是干脆就su切换root用户运行了(笑)
- 第二个坑是用户隔离时，没记错的话组ID还留有一些问题没有解决，内外映射有些问题，没有找到好办法。
- 第三个坑!!!重点，这个坑卡了我将近两天，虽然他并不复杂(没错就是我菜)。这个坑就是文件挂载。我一度怀疑是我代码有误，最后发现是因为k同学给的rootfs中没有bash。。图方便我就从别处拉了一份rootfs文件系统。。
- 第四个也是最大的遗憾了，就是网络隔离。在与它怼了2天后以我的失败告终。基础的网络隔离是实现了，运用了虚拟网桥。但是不知道是什么问题，我并没有实现端口映射，容器内无法访问外网。
- emm最后就是用了很多system。。。好像有点丢人orz

# The End