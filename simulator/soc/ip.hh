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
    IP_TYPE_RAM = 1,
    IP_TYPE_PERIPHERAL = 2,
    IP_TYPE_OTHER = 3,
    IP_TYPE_MAX
};

class baseBus; // Forward declaration

class base_ip {
public:
    base_ip(baseBus *bus, uint64_t id, IP_TYPE type,
            uint64_t base_address, uint64_t size,
            uint64_t irq_vec_start, uint64_t irq_vector_cnt);
    virtual ~base_ip() = default;
    
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

    // for ip that support shared memory, return a pointer to the shared memory for fast access.
    void *mem_master_get_shm_ptr(uint64_t addr)
    {
        return shm_ptr;
    }

    // Check if the IP can respond to an IRQ with a specific vector.
    // id: the ID of the IP that is sending the IRQ.
    // vector: the IRQ vector number.
    // Returns true if the IP can respond to the IRQ, false otherwise.
    // This function checks if the ID matches and if the vector is within the valid range.
    // The valid range is from vector_start to vector_start + nr_vectors.
    // This is used to determine if the IP should handle the IRQ.
    // The vector_start and nr_vectors are set during the IP's construction.
    // This function is thread-safe and can be called from multiple threads.
    // It uses a mutex to ensure that the ID and vector checks are atomic.
    // This is important for ensuring that the IP can handle IRQs correctly in a multi-thread
    bool irq_can_resp(uint64_t id, uint64_t vector)
    {
        return (id == this->id && \
            vector >= this->vector_start && vector < this->vector_start + this->nr_vectors);
    }

    virtual void handle_irq(uint64_t vector)
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

public:
    enum IP_TYPE ip_type; // Default type, can be set in derived classes
    uint64_t id;
    uint64_t base_addr;
    uint64_t addr_size;

    uint64_t vector_start;
    uint64_t nr_vectors;

    void *shm_ptr = NULL; // Pointer to shared memory, if applicable

private:
    std::mutex mtx;

protected:
    baseBus *bus; // Pointer to the bus this IP is connected to
};

#endif // IP_HH