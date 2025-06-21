#include "cosim_bridge.hh"

void cosim_bridge::mem_slave_read(uint64_t offset, uint64_t size, uint64_t *data)
{
    std::cout << "read addr:" << std::hex << offset << " size:" << size << std::endl;
    if (offset == 0)
        mem_master_read(35 * 0x1000, size, data);

    if (offset == 4)
        post_irq(5, 15);
}

void cosim_bridge::mem_slave_write(uint64_t offset, uint64_t size, uint64_t *data)
{
    std::cout << "write addr:" << std::hex << offset << " size:" << size << std::endl;
}

void cosim_bridge::handle_irq(uint64_t vector)
{
    std::cout << "handle irq: vector:" << std::hex << vector << std::endl;
}