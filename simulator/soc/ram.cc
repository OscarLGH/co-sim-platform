#include "ram.hh"
#include <cstring>

void ram::mem_slave_read(uint64_t offset, uint64_t size, uint64_t *data)
{
    memcpy(data, (uint64_t *)((uint8_t *)shm_ptr + offset), size);
    std::cout << "ram read: offset: " << std::hex << offset 
              << " size: " << size 
              << " data: " << *data 
              << std::endl;
}

void ram::mem_slave_write(uint64_t offset, uint64_t size, uint64_t *data)
{
    memcpy((uint64_t *)((uint8_t *)shm_ptr + offset), data, size);
    std::cout << "ram write: offset: " << std::hex << offset 
              << " size: " << size 
              << " data: " << *data 
              << std::endl;
}