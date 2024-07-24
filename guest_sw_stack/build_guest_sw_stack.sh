cd busybox
make defconfig
make -j $(nproc)
#make install prefix=$(pwd)/../../test/arch/x86_64/rootfs/
make clean
make defconfig
make ARCH=riscv CROSS_COMPILE=riscv64-linux-gnu- -j $(nproc) 
make CONFIG_PREFIX=$(pwd)/../../test/arch/riscv64/rootfs/ install -j $(nproc) 