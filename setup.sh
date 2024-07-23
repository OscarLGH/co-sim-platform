printf "Initializing setup. After this, you may build QEMU.\n"

# Repository information
REPOSITORY_DIR=$(git rev-parse --show-toplevel)
REPOSITORY_NAME=$(basename $REPOSITORY_DIR)

cd qemu
git checkout stable-9.0
git reset --hard
cd ..
# Edit original build files
echo "source pciemu/Kconfig" >> qemu/hw/misc/Kconfig
echo "subdir('pciemu')" >> qemu/hw/misc/meson.build

# Create symbolic links to device files
ln -s $REPOSITORY_DIR/src/hw/pciemu/ $REPOSITORY_DIR/qemu/hw/misc/

# Create symbolic link to the pciemu_hw.h include file
# This will avoid changing the meson files to be able to find this include
#ln -s $REPOSITORY_DIR/include/hw/pciemu_hw.h $REPOSITORY_DIR/src/hw/$REPOSITORY_NAME/pciemu_hw.h

# Configure QEMU
cd qemu
git checkout stable-9.0
./configure \
	--disable-werror \
	--target-list=x86_64-softmmu,riscv64-softmmu \
	--disable-vnc

printf "\nSetup finished. You may now build QEMU (cd qemu && make)\n"

make -j $(nproc)