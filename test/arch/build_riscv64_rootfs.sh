cd riscv64

../build_initial_rootfs.sh

cd rootfs
cp -d /usr/riscv64-linux-gnu/lib/*.so* lib/
cp ../../../linux_drivers . -r