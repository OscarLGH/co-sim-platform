#include "ip.hh"
#include <iostream>

class test_device : public baseIp {
public:
    using baseIp::baseIp;

    void reset() override {
        std::cout << "reset...\n";
    }

    void slave_read(uint64_t addr, uint64_t size, uint64_t *data)
    {
        std::cout << "read addr:" << addr << " size:" << size << std::endl;
    }

    void slave_write(uint64_t addr, uint64_t size, uint64_t *data)
    {
        std::cout << "write addr:" << addr << " size:" << size << std::endl;
    }

};