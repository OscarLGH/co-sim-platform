#!/bin/sh
file_name="vdisk_root.img"
file_sectors=256
image_format="ext2"

rm vdisk_root.img
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
cp rootfs/* tmp -r
sync
umount tmp
rm -rf tmp
sudo losetup -d $free_loop

../../../simulator/qemu/build/qemu-system-riscv64 \
	-M virt \
	-smp 16 \
	-m 8G \
	-kernel Image \
	-drive id=disk1,file=vdisk_root.img,format=raw,if=virtio \
	-append "console=ttyS0 root=/dev/vda1" \
	-serial stdio \
	-device pciemu,has_soc_backend=true,qemu_req_fd_name="/tmp/qemu-fifo-req",qemu_resp_fd_name="/tmp/qemu-fifo-resp",soc_backend_shm_name="/gem5_share_memory" \
