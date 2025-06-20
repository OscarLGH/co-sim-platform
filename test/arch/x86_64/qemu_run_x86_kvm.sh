#!/bin/sh
file_name_root="vdisk_root.img"
file_sectors_root=128
image_format_root="ext4"

rm $file_name_root
if [ ! -f $file_name_root ]; then
	dd if=/dev/zero of=$file_name_root bs=1M count=$file_sectors_root
fi

mkdir tmp

cp rootfs/* tmp -r

mke2fs -L '' -N 0 -O ^64bit -d tmp -m 5 -r 1 -t $image_format_root $file_name_root $file_sectors_rootM

rm -rf tmp

../../../simulator/qemu/build/qemu-system-x86_64 \
	-enable-kvm \
	-cpu host \
	-smp 4,sockets=1,cores=2,threads=2 \
	-m 8G \
	-machine q35,accel=kvm,kernel-irqchip=split \
	-bios /usr/share/ovmf/OVMF.fd \
	-drive id=disk1,file=vdisk_root.img,format=raw,if=virtio \
	-kernel ESP/bzImage \
	-append "console=ttyS0,115200 root=/dev/vda" \
	-serial stdio \
	-device dw_axi_dmac,has_soc_backend=false \
	-device dw_axi_dmac,has_soc_backend=false \
	-device dw_axi_dmac,has_soc_backend=false \
	-device dw_axi_dmac,has_soc_backend=false \
	-device dw_axi_dmac,has_soc_backend=false \
	-device dw_axi_dmac,has_soc_backend=false \
	-device dw_axi_dmac,has_soc_backend=false \
	-device dw_axi_dmac,has_soc_backend=false \
	-device dw_axi_dmac,has_soc_backend=false \
	-device dw_axi_dmac,has_soc_backend=false \
	-device dw_axi_dmac,has_soc_backend=false \
	-device dw_axi_dmac,has_soc_backend=false \
	-device dw_axi_dmac,has_soc_backend=false \
	-device dw_axi_dmac,has_soc_backend=false \
	-device dw_axi_dmac,has_soc_backend=false \
	-device intel-iommu,intremap=on \
	-d guest_errors -D log.txt
