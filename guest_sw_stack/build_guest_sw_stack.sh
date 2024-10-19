cd busybox
#make defconfig
#make -j $(nproc)
make -j $(nproc) install
cp -r _install/* $(pwd)/../../test/arch/x86_64/rootfs/ 
#make install prefix=$(pwd)/../../test/arch/x86_64/rootfs/
make clean
#make defconfig
make ARCH=riscv CROSS_COMPILE=riscv64-linux-gnu- -j $(nproc) install
cp -r _install/* $(pwd)/../../test/arch/riscv64/rootfs/ 
