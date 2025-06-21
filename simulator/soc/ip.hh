#ifndef IP_HH
#define IP_HH 

#include <cstdint>
#include <iostream>
#include <thread>
#include <mutex>

#define MMIO_ACCESS_RW_R 0
#define MMIO_ACCESS_RW_W 1

enum BUS_ACCESS_CODE {
    /* Remote port responses. */
    ACCESS_OK                =  0x0,
    ACCESS_DENIED            =  0x1,
    ACCESS_ADDR_ERROR        =  0x2,
    RP_RESP_MAX              =  0xF,
};

enum IP_TYPE {
    IP_TYPE_DRAM = 0,
    IP_TYPE_PERIPHERAL = 1,
    IP_TYPE_OTHER = 2,
    IP_TYPE_MAX
};

class baseBus; // Forward declaration

class baseIp {
public:
    baseIp(baseBus *bus, uint64_t id,
            uint64_t base_address, uint64_t size,
            uint64_t irq_vec_start, uint64_t irq_vector_cnt)
    {
        std::cout << "conducting ip, id:" << id \
        << " base: 0x" << std::hex << base_address << " size:"<< size \
        << " irq_vec_start:" << irq_vec_start << " irq_vector_cnt:"<< irq_vector_cnt \
        <<std::endl;

        this->base_addr = base_address;
        this->addr_size = size;
        this->bus = bus;
        this->id = id;
        this->vector_start = irq_vec_start;
        this->nr_vectors = irq_vector_cnt;
    }

    virtual ~baseIp() = default;
    
    bool mem_slave_addr_check(uint64_t addr)
    {
        return (addr >= base_addr && addr < base_addr + addr_size);
    }

    virtual void reset() = 0;

    BUS_ACCESS_CODE memaddr_can_access(bool rw, uint64_t offset, uint64_t size)
    {
        return ACCESS_OK;
    }

    // slave access, global address.
    int mem_slave_access(bool rw, uint64_t addr, uint64_t size, uint64_t *data)
    {
        uint64_t offset = addr - base_addr;
        BUS_ACCESS_CODE ret = ACCESS_OK;
        ret = memaddr_can_access(rw, offset, size);
        if (ret == ACCESS_OK) {
            mtx.lock();
            if (rw == MMIO_ACCESS_RW_R) {
                mem_slave_read(offset, size, data);
            } else {
                mem_slave_write(offset, size, data);
            }
            mtx.unlock();
        }

        return (int)ret;
    }

    // slave access, local addr(offset)
    virtual void mem_slave_read(uint64_t offset, uint64_t size, uint64_t *data) = 0;
    virtual void mem_slave_write(uint64_t offset, uint64_t size, uint64_t *data) = 0;

    // for ip to access bus, global address.
    void mem_master_read(uint64_t addr, uint64_t size, uint64_t *data);
    void mem_master_write(uint64_t addr, uint64_t size, uint64_t *data);

    bool irq_can_resp(uint64_t id, uint64_t vector)
    {
        return (id == this->id && \
            vector >= this->vector_start && vector < this->vector_start + this->nr_vectors);
    }

    void handle_irq(uint64_t vector)
    {
        std::cout << "ip:" << id \
        << " handle irq vector " << vector \
        << std::endl;
    }

    void recv_irq(uint64_t id, uint64_t vector)
    {
        if (irq_can_resp(id, vector))
            handle_irq(vector);
    }

    void post_irq(uint64_t id, uint64_t vector);

private:
    enum IP_TYPE ip_type; // Default type, can be set in derived classes
    std::mutex mtx;

    uint64_t id;
    uint64_t base_addr;
    uint64_t addr_size;

    uint64_t vector_start;
    uint64_t nr_vectors;

protected:
    baseBus *bus; // Pointer to the bus this IP is connected to
};

#endif // IP_HH