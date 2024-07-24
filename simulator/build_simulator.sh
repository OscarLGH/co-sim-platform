# Configure QEMU
cd qemu
git checkout stable-9.0
./configure \
	--disable-werror \
	--target-list=x86_64-softmmu,riscv64-softmmu \
	--disable-vnc

make -j $(nproc)

cd ../gem5
scons build/RISCV/gem5.opt -j $(nproc)