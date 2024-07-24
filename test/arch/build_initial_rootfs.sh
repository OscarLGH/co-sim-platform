cd rootfs
mkdir dev etc lib usr var proc tmp home root mnt sys

rm -rf init
ln -sv bin/busybox init
rm linuxrc

cd dev
sudo mknod console c 5 1
sudo mknod null c 1 3
cd ..
cd etc
echo "proc /proc proc defaults 0 0" >> fstab
echo "sysfs /sys sysfs defaults 0 0" >> fstab

mkdir init.d
echo "#!/bin/sh" >> init.d/rcS
echo "mount -a" >> init.d/rcS

chmod +x init.d/rcS

echo "::sysinit:/etc/init.d/rcS" >> inittab
echo "console::respawn:-/bin/sh" >> inittab
echo "::ctrlaltdel:/sbin/reboot" >> inittab
echo "::shutdown:/bin/umount -a -r" >> inittab

cd ..

