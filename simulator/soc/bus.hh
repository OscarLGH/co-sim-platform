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
        std::cout << "master_read addr: " << addr << " size: " << size << std::endl;

        for (auto &ip : ipList) {
            if (ip->addr_check_access(addr)) {
                ip->ip_access(MMIO_ACCESS_RW_R, addr, size, data);
                return;
            }
        }
        std::cout << "No IP found for address: " << addr << std::endl;
    }

    void master_write(uint64_t addr, uint64_t size, uint64_t *data)
    {
        std::cout << "master_write addr: " << addr << " size: " << size << std::endl;

        for (auto &ip : ipList) {
            if (ip->addr_check_access(addr)) {
                ip->ip_access(MMIO_ACCESS_RW_W, addr, size, data);
                return;
            }
        }
        std::cout << "No IP found for address: " << addr << std::endl;
    }
    //virtual void post_irq();
private:
    std::mutex mtx;
    std::list<baseIp *> ipList;
};

#endif // BUS_HH