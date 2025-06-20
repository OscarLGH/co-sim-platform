#include "ip.hh"
#include "bus.hh"

void baseIp::master_read(uint64_t addr, uint64_t size, uint64_t *data)
{
    if (bus) {
        bus->master_read(addr, size, data);
    } else {
        std::cerr << "Error: No bus connected to this IP." << std::endl;
    }
}

void baseIp::master_write(uint64_t addr, uint64_t size, uint64_t *data)
{
    if (bus) {
        bus->master_write(addr, size, data);
    } else {
        std::cerr << "Error: No bus connected to this IP." << std::endl;
    }
}