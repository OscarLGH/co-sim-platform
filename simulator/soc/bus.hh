#ifndef BUS_HH
#define BUS_HH 

#include <cstdint>
#include <iostream>
#include <thread>
#include <mutex>
#include <list>

#include "ip.hh"

#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

#include <string.h>

class base_ip; // Forward declaration
class baseBus {
public:

    baseBus(int id, char *name) : bus_id(id) {
        LOG_DEBUG("baseBus constructed with ID: %d, name: %s", id, name);
        shm_fd = shm_open(name, O_CREAT | O_RDWR, 0666);
    }

    void connect_ip(base_ip *ip)
    {
        ipList.push_back(ip);
        if (ip->ip_type == IP_TYPE_RAM) {
            LOG_DEBUG("Connecting IP with ID: %lu, type: %d, base_addr: %lx, addr_size: %lx",
                      ip->id, ip->ip_type, ip->base_addr, ip->addr_size);
            ftruncate(shm_fd, ip->base_addr + ip->addr_size);
            shm_ptr = mmap(NULL, ip->base_addr + ip->addr_size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
            if (shm_ptr == MAP_FAILED) {
                LOG_ERROR("Failed to map shared memory: %s", strerror(errno));
                shm_ptr = nullptr; // Set to nullptr on failure
            } else {
                ip->shm_ptr = ((char *)shm_ptr + ip->base_addr); // Set the shared memory pointer in the IP
                LOG_INFO("Shared memory mapped at: %p for IP with ID: %lu", ip->shm_ptr, ip->id);
            }        
        }
    }

    void master_read(uint64_t addr, uint64_t size, uint64_t *data)
    {
        LOG_DEBUG("master_read addr: %lx size: %lu", addr, size);

        for (auto &ip : ipList) {
            if (ip->mem_slave_addr_check(addr)) {
                ip->mem_slave_access(MMIO_ACCESS_RW_R, addr, size, data);
                return;
            }
        }
        LOG_ERROR("No IP found for address: %lx", addr);
    }

    void master_write(uint64_t addr, uint64_t size, uint64_t *data)
    {
        LOG_DEBUG("master_write addr: %lx size: %lu data: %lx", addr, size, *data);

        for (auto &ip : ipList) {
            if (ip->mem_slave_addr_check(addr)) {
                ip->mem_slave_access(MMIO_ACCESS_RW_W, addr, size, data);
                return;
            }
        }
        LOG_ERROR("No IP found for address: %lx", addr);
    }

    // For IPs that support shared memory, return a pointer to the shared memory for fast access.
    // This function checks all IPs to find the one that can handle the address.
    // If no IP can handle the address, it returns nullptr.
    // This is useful for IPs that need to access shared memory directly without going through the bus.
    // It allows for faster access to shared memory regions.
    // @addr should be a global address that the IP can handle.
    // Returns a pointer to the start of IP's shared memory if found, otherwise returns nullptr.
    void *master_get_shm_ptr(uint64_t addr)
    {
        for (auto &ip : ipList) {
            if (ip->mem_slave_addr_check(addr) && ip->ip_type == IP_TYPE_RAM) {
                return ((char *)shm_ptr + addr);
            }
        }
        LOG_ERROR("No IP found for shared memory address: %lx", addr);
        return nullptr;
    }
    
    void post_irq(uint64_t id, uint64_t vector)
    {
        LOG_DEBUG("Posting IRQ: id = %lu, vector = %lu", id, vector);
        for (auto &ip : ipList) {
            if (ip->irq_can_resp(id, vector)) {
                ip->recv_irq(id, vector);
                return;
            }
        }
        LOG_ERROR("No IP can respond to IRQ: id = %lu, vector = %lu", id, vector);
    }

private:
    int bus_id; // Unique ID for the bus, can be used for debugging or identification.
    std::mutex mtx;
    std::list<base_ip *> ipList;
    int shm_fd; // File descriptor for shared memory.
    void *shm_ptr; // Pointer to shared memory, if applicable.
};

#endif // BUS_HH