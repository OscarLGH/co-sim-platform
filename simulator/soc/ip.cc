#include "ip.hh"
#include "bus.hh"

base_ip::base_ip(baseBus *bus, uint64_t id, IP_TYPE type,
            uint64_t base_address, uint64_t size,
            uint64_t irq_vec_start, uint64_t irq_vector_cnt)
{
    LOG_DEBUG("ip constructed.\
        id = %d, type = %d, base_addr = %lx, size = %lx, irq_vec_start = %lx, irq_vector_cnt = %lx\n",
        id, type, base_address, size, irq_vec_start, irq_vector_cnt
    );

    this->base_addr = base_address;
    this->addr_size = size;
    this->bus = bus;
    this->id = id;
    this->vector_start = irq_vec_start;
    this->nr_vectors = irq_vector_cnt;
    this->ip_type = type; // Default type, can be set in derived classes
    this->bus->connect_ip(this);
}

void base_ip::mem_master_read(uint64_t addr, uint64_t size, uint64_t *data)
{
    if (bus) {
        bus->master_read(addr, size, data);
    } else {
        std::cerr << "Error: No bus connected to this IP." << std::endl;
    }
}

void base_ip::mem_master_write(uint64_t addr, uint64_t size, uint64_t *data)
{
    if (bus) {
        bus->master_write(addr, size, data);
    } else {
        std::cerr << "Error: No bus connected to this IP." << std::endl;
    }
}

void base_ip::post_irq(uint64_t id, uint64_t vector)
{
    if (bus) {
        bus->post_irq(id, vector);
    } else {
        std::cerr << "Error: No bus connected to this IP." << std::endl;
    }
}