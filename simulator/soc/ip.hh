#ifndef IP_HH
#define IP_HH 

#include <cstdint>
#include <iostream>
#include <thread>
#include <mutex>

#include "debugger.hh"

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

class base_bus; // Forward declaration

class base_ip {
public:
    // Constructor for base_ip, initializes the IP with a bus, ID, type, base address, size,
    // IRQ vector start, and number of IRQ vectors.
    // @bus: Pointer to the bus this IP is connected to.
    // @id: Unique identifier for the IP.
    // @type: Type of the IP (e.g., RAM, peripheral).
    // @base_address: Base address of the IP in the bus memory map.
    // @size: Size of the IP's memory region.
    // @irq_vec_start: Starting IRQ vector number for this IP.
    // @irq_vector_cnt: Number of IRQ vectors this IP can handle.
    // This constructor initializes the IP's base address, size, ID, type, and IRQ vector information.
    // It also connects the IP to the bus by calling the bus's connect_ip method.
    base_ip(base_bus *bus, uint64_t id, IP_TYPE type,
            uint64_t base_address, uint64_t size,
            uint64_t irq_vec_start, uint64_t irq_vector_cnt);

    // Destructor for base_ip, cleans up the IP.
    // It is declared virtual to allow derived classes to override it.
    virtual ~base_ip() = default;
    
    // Check if the given address is within the IP's memory range.
    // @addr: The address to check.
    // Returns true if the address is within the range [base_addr, base_addr + addr_size),
    // false otherwise.
    bool mem_slave_addr_check(uint64_t addr)
    {
        return (addr >= base_addr && addr < base_addr + addr_size);
    }

    // Reset the IP to its initial state.
    // This function is pure virtual, meaning derived classes must implement it.
    // It is used to reset the IP's internal state, memory, and any other resources
    // to their initial values.
    virtual void reset() = 0;

    // Check if the given address can be accessed for a specific operation.
    // @rw: The type of access (read or write).
    // @offset: The offset from the base address to check.
    // @size: The size of the access.
    // Returns a BUS_ACCESS_CODE indicating whether the access is allowed or not.
    virtual BUS_ACCESS_CODE memaddr_can_access(bool rw, uint64_t offset, uint64_t size)
    {
        return ACCESS_OK;
    }

    // Slave access function for memory operations.
    // This function checks if the given address can be accessed for a read or write operation.
    // @rw: The type of access (read or write).
    // @addr: The global address to access.
    // @size: The size of the access.
    // @data: Pointer to the data buffer to read or write.
    // Returns an integer indicating the result of the access operation.
    // If the access is allowed, it locks the mutex, performs the read or write operation,
    // and then unlocks the mutex. If the access is denied, it returns an error code.
    // This function is thread-safe and ensures that only one thread can access the memory
    // at a time by using a mutex.
    int mem_slave_access(bool rw, uint64_t addr, uint64_t size, void *data)
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

    // Slave memory read and write functions.
    // These functions are pure virtual, meaning derived classes must implement them.
    // They are used to read from and write to the IP's memory region.
    // @offset: The offset from the base address to read or write.
    // @size: The size of the data to read or write.
    // @data: Pointer to the data buffer to read or write.
    // These functions are called by the bus when an IP is accessed for a read or write operation.
    // They should handle the actual memory access logic, such as reading from or writing to
    // the IP's internal memory or shared memory region.
    // The derived classes should implement these functions to perform the actual memory operations.
    virtual void mem_slave_read(uint64_t offset, uint64_t size, void *data) = 0;
    virtual void mem_slave_write(uint64_t offset, uint64_t size, void *data) = 0;

    // Master memory read and write functions.
    // These functions are used by the IP to read from and write to the bus.
    // They take a global address and size, and perform the read or write operation.
    // @addr: The global address to read from or write to.
    // @size: The size of the data to read or write.
    // @data: Pointer to the data buffer to read or write.
    // These functions are called by the IP when it needs to access memory on the bus.
    // They should call the bus's master_read or master_write functions to perform the operation.
    // If the bus is not connected, they log an error message.
    // The derived classes can use these functions to access memory on the bus without
    // needing to know the details of the bus implementation.
    void mem_master_read(uint64_t addr, uint64_t size, void *data);
    void mem_master_write(uint64_t addr, uint64_t size, void *data);

    // Get a pointer to the shared memory region for fast access.
    // This function is used by the IP to access shared memory directly without going through the bus.
    // It checks all IPs to find the one that can handle the address.
    // If no IP can handle the address, it returns nullptr.
    // @addr: The global address to check.
    // Returns a pointer to the start of the IP's shared memory if found, otherwise returns nullptr.
    // This is useful for IPs that need to access shared memory regions directly for performance
    // reasons, such as RAM or other memory-mapped devices.
    // If the IP type is not RAM, it returns nullptr.
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

    // Handle an IRQ with a specific vector.
    // It is called when an IRQ is received for this IP.
    // @vector: The IRQ vector number to handle.
    // The derived classes should implement this function to handle the IRQ appropriately,
    virtual void handle_irq(uint64_t vector)
    {
        LOG_DEBUG("IP %lu handling IRQ vector %lu", id, vector);
    }

    // This function is called by the bus when an IRQ is received for this IP.
    // It checks if the IP can respond to the IRQ using the irq_can_resp function.
    // If the IP can respond, it calls the handle_irq function to handle the IRQ.
    // @id: The ID of the IP that is sending the IRQ.
    // @vector: The IRQ vector number to handle.
    void recv_irq(uint64_t id, uint64_t vector)
    {
        if (irq_can_resp(id, vector))
            handle_irq(vector);
    }

    // Post an IRQ to the bus.
    // This function is called by the IP to post an IRQ to the bus.
    // It uses the bus's post_irq function to send the IRQ to the bus.
    // @id: The ID of the IP that is sending the IRQ.
    // @vector: The IRQ vector number to post.
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
    base_bus *bus; // Pointer to the bus this IP is connected to
};

#endif // IP_HH