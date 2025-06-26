#include "soc_top.hh"
#include "bus.hh"
#include "cosim_bridge.hh"
#include "ram.hh"
#include "debugger.hh"

int main() {
    uint64_t i = 0, j = 0;

    debugger::set_level(debugger::DEBUG);

    base_bus *bus = new base_bus(0, "soc_bus");
    ram *dev[32];
    for (i = 0; i < 4; i++) {
        for (j = 1; j <=8; j++) {
            dev[i] = new ram(bus, i, (i << 38) | (j << 34), 0x1000000, 0, 0);
        }
        
    }
    cosim_bridge *co_bridge = new cosim_bridge(bus, i, 0, 0, 0, 1024,
        "./fifo/qemu_to_soc_req",
        "./fifo/qemu_to_soc_resp",
        "./fifo/soc_to_qemu_req",
        "./fifo/soc_to_qemu_resp"
    );

    co_bridge->cosim_start_polling_remote();

    while(1) {
        //LOG_DEBUG("idle");
    }
    return 0;
}