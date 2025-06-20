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

class baseIp {
public:
    baseIp(uint64_t base_address, uint64_t size)
    {
        std::cout << "conducting ip, base:" << base_address << " size:"<< size <<std::endl;
        base_addr = base_address;
        addr_size = size;
    }

    virtual ~baseIp() = default;
    
    bool addr_check_access(uint64_t addr)
    {
        std::cout << "addr check hit:" << addr << std::endl;
        return (addr >= base_addr && addr < base_addr + addr_size);
    }

    virtual void reset() = 0;

    BUS_ACCESS_CODE mmio_can_access(bool rw, uint64_t offset, uint64_t size)
    {
        return ACCESS_OK;
    }

    int ip_access(bool rw, uint64_t addr, uint64_t size, uint64_t *data)
    {
        uint64_t offset = addr - base_addr;
        BUS_ACCESS_CODE ret = ACCESS_OK;
        ret = mmio_can_access(rw, offset, size);
        if (ret == ACCESS_OK) {
            mtx.lock();
            if (rw == MMIO_ACCESS_RW_R) {
                slave_read(addr, size, data);
            } else {
                slave_write(addr, size, data);
            }
            mtx.unlock();
        }

        return (int)ret;
    }

    virtual void slave_read(uint64_t addr, uint64_t size, uint64_t *data) = 0;
    virtual void slave_write(uint64_t addr, uint64_t size, uint64_t *data) = 0;

    //void master_read(uint64_t addr, uint64_t size, uint64_t *data);
    //void master_write(uint64_t addr, uint64_t size, uint64_t *data);
    //virtual void post_irq();
private:
    std::mutex mtx;
    uint64_t base_addr;
    uint64_t addr_size;
};