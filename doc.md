### 运行环境

* 考核附件的简单文件系统，做了适当修改
> 1.因为chroot时出现问题： 
"chroot: failed to run command ‘/bin/bash’: No such file or directory"
解决方法： cp /bin/bash rootfs/bin/bash
复制之后发现还是不行，查了下依赖库还有问题。
解决方法：ldd /bin/bash
然后把这些库复制到简易文件系统中相应位置即可
 >
 2.创建proc文件夹并进行挂载
 mkdir rootfs/proc
 mount -t proc proc roofs/proc
 
### 完成程度
> 1. 文件系统隔离
>
2. 进程隔离
>
3. 网络隔离
>
4. 用户隔离


### 拓展功能
> 1. UTS隔离
>
2. IPC隔离
>
3. 容器资源限制（最简易

### 已知问题
第一次运行的时候可能会ping不同，但exit后再启动就能够ping通了。
>
然后那个端口映射查了一下是这两条命令没错，但跑的时候出错了。

### 示例图
![示例](https://note.youdao.com/yws/api/personal/file/9D66A2A732AC4126B568DD74AD6A06A3?method=download&shareKey=09bad37a35c4ebb8ef3a70cadd5d82f0 "示例")
第一个分界上方是父进程的相关信息
>
中间部分是新name space信息以及ping测试
>
下面就是一些信息修改的证实









