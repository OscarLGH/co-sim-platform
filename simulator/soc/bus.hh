#ifndef BUS_HH
#define BUS_HH 

#include <cstdint>
#include <iostream>
#include <thread>
#include <mutex>
#include <list>

#include "ip.hh"

class baseIp; // Forward declaration
class baseBus {
public:
    void connect_ip(baseIp *ip)
    {
        ipList.push_back(ip);
    }

    void master_read(uint64_t addr, uint64_t size, uint64_t *data)
    {
        std::cout << "master_read addr: " << std::hex << addr << " size: " << size << std::endl;

        for (auto &ip : ipList) {
            if (ip->mem_slave_addr_check(addr)) {
                ip->mem_slave_access(MMIO_ACCESS_RW_R, addr, size, data);
                return;
            }
        }
        std::cout << "No IP found for address: " << std::hex << addr << std::endl;
    }

    void master_write(uint64_t addr, uint64_t size, uint64_t *data)
    {
        std::cout << "master_write addr: " << std::hex << addr << " size: " << size << std::endl;

        for (auto &ip : ipList) {
            if (ip->mem_slave_addr_check(addr)) {
                ip->mem_slave_access(MMIO_ACCESS_RW_W, addr, size, data);
                return;
            }
        }
        std::cout << "No IP found for address: " << std::hex << addr << std::endl;
    }
    
    void post_irq(uint64_t id, uint64_t vector)
    {
        std::cout << "IRQ vector:" << vector << std::endl;
        for (auto &ip : ipList) {
            if (ip->irq_can_resp(id, vector)) {
                ip->recv_irq(id, vector);
                return;
            }
        }
        std::cout << "No IP found for irq: " << vector << std::endl;
    }

private:
    std::mutex mtx;
    std::list<baseIp *> ipList;
};

#endif // BUS_HH