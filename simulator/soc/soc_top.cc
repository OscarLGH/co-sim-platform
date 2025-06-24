#include "soc_top.hh"
#include "bus.hh"
#include "cosim_bridge.hh"
#include "ram.hh"
#include "debugger.hh"

int main() {
    int i = 0;

    debugger::set_level(debugger::DEBUG);

    baseBus *bus = new baseBus(0, "soc_bus");
    ram *dev[32];
    for (i = 0; i < 32; i++) {
        dev[i] = new ram(bus, i, i * 0x1000, 0x1000, 0, 0);
    }
    cosim_bridge *co_bridge = new cosim_bridge(bus, i, 0, 0, 0, 1024,
        "./fifo/qemu_to_soc_req",
        "./fifo/qemu_to_soc_resp",
        "",
        ""
    );

    co_bridge->cosim_start_polling_remote();

    base_ip *ip;
    uint64_t data = 0;
    bus->master_read(0x1000, 4, &data);
    bus->master_read(0x1004, 4, &data);
    data = 0x55aaaa55;
    bus->master_write(0x1000, 4, &data);
    bus->post_irq(5, 16);

    void *ptr = bus->master_get_shm_ptr(0x1000);
    memset(ptr, 0xaa, 0x1000);

    while(1);
    return 0;
}