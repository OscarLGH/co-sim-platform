#include "ip.hh"
#include "bus.hh"

void baseIp::mem_master_read(uint64_t addr, uint64_t size, uint64_t *data)
{
    if (bus) {
        bus->master_read(addr, size, data);
    } else {
        std::cerr << "Error: No bus connected to this IP." << std::endl;
    }
}

void baseIp::mem_master_write(uint64_t addr, uint64_t size, uint64_t *data)
{
    if (bus) {
        bus->master_write(addr, size, data);
    } else {
        std::cerr << "Error: No bus connected to this IP." << std::endl;
    }
}

void baseIp::post_irq(uint64_t id, uint64_t vector)
{
    if (bus) {
        bus->post_irq(id, vector);
    } else {
        std::cerr << "Error: No bus connected to this IP." << std::endl;
    }
}