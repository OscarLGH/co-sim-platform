#include "ip.hh"
#include <iostream>

class test_device : public base_ip {
public:
    using base_ip::base_ip;

    void reset() override {
        std::cout << "reset...\n";
    }

    void mem_slave_read(uint64_t offset, uint64_t size, uint64_t *data)
    {
        std::cout << "read addr:" << std::hex << offset << " size:" << size << std::endl;
        if (offset == 0)
            mem_master_read(35 * 0x1000, size, data);

        if (offset == 4)
            post_irq(5, 15);
    }

    void mem_slave_write(uint64_t offset, uint64_t size, uint64_t *data)
    {
        std::cout << "write addr:" << std::hex << offset << " size:" << size << std::endl;
    }

};