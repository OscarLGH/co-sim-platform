#!/bin/sh
file_name="vdisk.img"
file_sectors=16
image_format="vfat"

rm vdisk.img
if [ ! -f $file_name ]; then
	dd if=/dev/zero of=$file_name bs=1M count=$file_sectors
	sleep 1
	sudo fdisk $file_name << FCMD
	n
	p
	
	
	
	w
FCMD
fi

free_loop=`losetup -f`
losetup $free_loop $file_name
kpartx -av $free_loop
mount_dev=${free_loop##*/}
mount_path="/dev/mapper/"${mount_dev}"p1"
sleep 1
mkfs.$image_format $mount_path
mkdir tmp
mount $mount_path tmp
cp ESP/* tmp -r
sync
umount tmp
rm -rf tmp
sudo losetup -d $free_loop

qemu-system-x86_64 \
	-cpu Broadwell \
	-smp 4,sockets=1,cores=2,threads=2 \
	-m 2G \
	-bios /usr/share/OVMF/OVMF_CODE.fd \
	-drive id=disk,file=vdisk.img,format=raw,if=none \
	-device ahci,id=ahci \
	-device ide-drive,drive=disk,bus=ahci.0 \
	-serial stdio
