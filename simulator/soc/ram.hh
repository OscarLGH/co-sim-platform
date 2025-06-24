#include "ip.hh"
#include <iostream>

class ram : public base_ip {
public:
    using base_ip::base_ip;

    ram(base_bus *bus, uint64_t id,
        uint64_t base_address, uint64_t size,
        uint64_t irq_vec_start, uint64_t irq_vector_cnt)
            : base_ip(bus, id, IP_TYPE_RAM, base_address, size, irq_vec_start, irq_vector_cnt)
            {
            }

    void reset() override {
        LOG_DEBUG("ram reset called.");
    }

    void mem_slave_read(uint64_t offset, uint64_t size, uint64_t *data);
    void mem_slave_write(uint64_t offset, uint64_t size, uint64_t *data);
};