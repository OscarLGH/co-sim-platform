export KERNELDIR=$(pwd)/../../guest_sw_stack/linux/

cd pci_driver_model
make -j $(nproc)
cd ..

cd dw_edma
make -j $(nproc)
cd ..

cd dts-platform-driver-model
make -j $(nproc)
cd ..