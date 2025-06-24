#include "ram.hh"
#include <cstring>

void ram::mem_slave_read(uint64_t offset, uint64_t size, uint64_t *data)
{
    memcpy(data, (uint64_t *)((uint8_t *)shm_ptr + offset), size);
    LOG_DEBUG("ram read: offset: %lx size: %lx data: %lx", 
              offset, size, *data);
}

void ram::mem_slave_write(uint64_t offset, uint64_t size, uint64_t *data)
{
    memcpy((uint64_t *)((uint8_t *)shm_ptr + offset), data, size);
    LOG_DEBUG("ram write: offset: %lx size: %lx data: %lx", 
              offset, size, *data);
}