echo "input rootfs path (/tmp/newroot)"
read path
echo "container path is ${path}?"
mkdir $path

tar -xf rootfs.tar -C $path
mkdir $path/proc

cp /bin/bash $path/bin
#ldd $path/bin/bash
cp /lib/x86_64-linux-gnu/libtinfo.so.5 $path/lib
cp /lib/x86_64-linux-gnu/libdl.so.2 $path/lib

echo "rootfs,lib,Done"
mkdir $path/SERVER
cp ./HTTPSVR $path/SERVER

echo "netdemo Done"
