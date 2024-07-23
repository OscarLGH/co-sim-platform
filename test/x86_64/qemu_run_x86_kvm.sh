#!/bin/sh
file_name_esp="vdisk_esp.img"
file_sectors_esp=16
image_format_esp="vfat"

file_name_root="vdisk_root.img"
file_sectors_root=128
image_format_root="ext2"

rm $file_name_esp
if [ ! -f $file_name_esp ]; then
	dd if=/dev/zero of=$file_name_esp bs=1M count=$file_sectors_esp
	sleep 1
	sudo fdisk $file_name_esp << FCMD
	n
	p
	
	
	
	w
FCMD
fi

rm $file_name_root
if [ ! -f $file_name_root ]; then
	dd if=/dev/zero of=$file_name_root bs=1M count=$file_sectors_root
	sleep 1
	sudo fdisk $file_name_root << FCMD
	n
	p
	
	
	
	w
FCMD
fi

free_loop=`losetup -f`
losetup $free_loop $file_name_esp
kpartx -av $free_loop
mount_dev=${free_loop##*/}
mount_path="/dev/mapper/"${mount_dev}"p1"

sleep 1
mkfs.$image_format_esp $mount_path
mkdir tmp
mount $mount_path tmp
cp ESP/* tmp -r
sync
umount tmp

free_loop=`losetup -f`
losetup $free_loop $file_name_root
kpartx -av $free_loop
mount_dev=${free_loop##*/}
mount_path="/dev/mapper/"${mount_dev}"p1"

sleep 1
mkfs.$image_format_root $mount_path

mount $mount_path tmp
cp rootfs/* tmp -r
sync
umount tmp
rm -rf tmp

sudo losetup -d $free_loop

../../qemu/build/qemu-system-x86_64 \
	-enable-kvm \
	-cpu host\
	-smp 4,sockets=1,cores=2,threads=2 \
	-m 2G \
	-bios /usr/share/OVMF/OVMF_CODE.fd \
	-drive id=disk0,file=vdisk_esp.img,format=raw,if=none \
	-device ahci,id=ahci0 \
	-device ide-hd,drive=disk0,bus=ahci0.0 \
	-drive id=disk1,file=vdisk_root.img,format=raw,if=none \
	-device ide-hd,drive=disk1,bus=ahci0.1 \
	-device pciemu,id=pciemu0 \
	-serial stdio
