#include "soc_top.hh"
#include "bus.hh"
#include "test_device.hh"

int main() {
    int i = 0;
    baseBus *bus = new baseBus();
    test_device *dev[32];
    for (i = 0; i < 32; i++) {
        dev[i] = new test_device(bus, i, i * 0x1000, 0x1000, 0, 32);
    }

    for (i = 0; i < 32; i++) {
        bus->connect_ip(dev[i]);
    }

    baseIp *ip;
    uint64_t data = 0x55aaaa55;
    bus->master_read(0x1000, 4, &data);
    bus->master_read(0x1004, 4, &data);
    bus->post_irq(5, 16);
    return 0;
}