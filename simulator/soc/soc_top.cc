#include "soc_top.hh"
#include "bus.hh"
#include "cosim_bridge.hh"
#include "test_device.hh"

int main() {
    int i = 0;
    baseBus *bus = new baseBus();
    test_device *dev[32];
    for (i = 0; i < 32; i++) {
        dev[i] = new test_device(bus, i, i * 0x1000, 0x1000, 0, 32);
    }
    cosim_bridge *co_bridge = new cosim_bridge(bus, i, 0, 0, 0, 1024,
        "./fifo/qemu_to_soc_req",
        "./fifo/qemu_to_soc_resp",
        "",
        ""
    );

    for (i = 0; i < 32; i++) {
        bus->connect_ip(dev[i]);
    }

    bus->connect_ip(co_bridge);

    co_bridge->cosim_start_polling_remote();

    base_ip *ip;
    uint64_t data = 0x55aaaa55;
    bus->master_read(0x1000, 4, &data);
    bus->master_read(0x1004, 4, &data);
    bus->post_irq(5, 16);
    return 0;
}