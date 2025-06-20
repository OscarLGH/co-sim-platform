#include "soc_top.hh"
#include "test_device.hh"

int main() {
    int i = 0;
    test_device *dev[32];
    for (i = 0; i < 32; i++) {
        dev[i] = new test_device(i, 0x1000ULL);
    }

    baseIp *ip;
    uint64_t data = 0x55aaaa55;
    for (i = 0; i < 32; i++) {
        ip = dev[0];
        ip->reset();
        ip->ip_access(MMIO_ACCESS_RW_R, i, 4, &data);
    }
    return 0;
}