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
class base_bus {
public:
    // Constructor for base_bus, initializes the bus with a unique ID and shared memory name.
    // @id: Unique identifier for the bus.
    // @name: Name for the shared memory segment, used for inter-process communication.
    // It creates a shared memory segment with the specified name.
    base_bus(int id, char *name) : bus_id(id) {
        LOG_DEBUG("base_bus constructed with ID: %d, name: %s", id, name);
        shm_fd = shm_open(name, O_CREAT | O_RDWR, 0666);
    }

    // Destructor for base_bus, cleans up the shared memory segment.
    // It closes the shared memory file descriptor and unlinks the shared memory segment.
    ~base_bus() {
        LOG_DEBUG("base_bus destructed with ID: %d", bus_id);
        if (shm_ptr) {
            munmap(shm_ptr, shm_fd);
            shm_ptr = nullptr; // Set to nullptr after unmapping
        }
        if (shm_fd >= 0) {
            close(shm_fd);
            shm_fd = -1; // Set to -1 after closing
        }
        shm_unlink("/soc_shm"); // Unlink the shared memory segment
        LOG_DEBUG("Shared memory segment unlinked.");
    }

    // Connects an IP to the bus.
    // @ip: Pointer to the base_ip object representing the IP to be connected.
    // It adds the IP to the list of connected IPs and maps the shared memory if the IP type is RAM.
    // It also sets the shared memory pointer in the IP to the start of the shared memory region.
    // If the IP type is not RAM, it does not map shared memory.
    // If the mapping fails, it logs an error and sets the shared memory pointer to nullptr.

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

    // Master read and write functions for the bus.
    // These functions are used by IPs to read from and write to the bus.
    // They take a global address and size, and perform the read or write operation.
    void master_read(uint64_t addr, uint64_t size, void *data)
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

    // This function writes data to a specific address on the bus.
    // It checks all connected IPs to find the one that can handle the address.
    // If an IP can handle the address, it performs a write operation on that IP.
    // @addr: The global address where the data should be written.
    // @size: The size of the data to be written.
    // @data: Pointer to the data buffer to be written.
    void master_write(uint64_t addr, uint64_t size, void *data)
    {
        LOG_DEBUG("master_write addr: %lx size: %lu", addr, size);

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
    
    // Posts an IRQ to the bus.
    // This function iterates through the list of connected IPs and checks if any IP can respond to the IRQ.
    // If an IP can respond, it calls the recv_irq method on that IP.
    // If no IP can respond, it logs an error message.
    // @id: The ID of the IP that is sending the IRQ.
    // @vector: The IRQ vector number.
    // This function is used to notify the bus that an IRQ has occurred and needs to be handled.
    // It is typically called by IPs when they need to signal an interrupt.
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