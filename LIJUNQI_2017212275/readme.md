# Poor Container

by 李俊琪 2017212275

为什么叫做poor container呢？因为它有如下两个严重bug：

+ uid可以映射成功，但是gid映射失败（gid显示65534）
+ 似乎User Namespace配置还有其他问题，容器内部虽然uid为0，但是无法使用rootfs/bin/ping （ping: permission denied (are you root?) )



### 实现功能

仅实现了chroot,  PID namespace, IPC namespace, User namespace. 看了一下Network namespace 了解了下原理感觉可以实现，但是ping不了那个Bug困扰了我好久，最终打消了我尝试实现的愿望......



### 使用

直接 ./container即可 不需要sudo root权限. (测试环境 Fedora 29)

使用了 capability.h 可能需要安装 libcap-dev (Debian/Ubuntu) / libcap-devel (RedHat)



### 参考

[DOCKER基础技术：LINUX NAMESPACE（下)](https://coolshell.cn/articles/17029.html) 

[DOCKER基础技术：LINUX NAMESPACE（上）](https://coolshell.cn/articles/17010.html)

[Namespaces in operation](https://lwn.net/Articles/531114/)   以及里面part1-7 一系列namespace的文章



### 感想

作为星期三考口语，而且几乎没学过C语言的菜鸡，当我看到这个题目的时候我是懵逼的。于是我只好导出查文章，查各种API的用法并从中汲(copy)取(and)知(paste)识。最后c语言还是处于懵逼状态，做的东西也没法让人满意。不过最大的收获是了解了从前看都看不下去的容器基础知识，明白了掌握基础知识的重要性。