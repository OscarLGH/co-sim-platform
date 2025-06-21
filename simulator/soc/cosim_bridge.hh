#include "ip.hh"
#include <iostream>

#include <cstdint>

enum exPktType {
      EX_PKT_RD = 0,
      EX_PKT_WR = 1,
      EX_PKT_IRQ = 2,
      EX_PKT_RESP_FLAG = 0x100
};

typedef struct exPktCmd {
    enum exPktType type;
    int length;
    uint64_t addr;
    uint64_t data;
} exPktCmd;

class cosim_bridge : public base_ip {
public:
    using base_ip::base_ip;

    void reset() override {
        std::cout << "reset...\n";
    }

    void mem_slave_read(uint64_t offset, uint64_t size, uint64_t *data);
    void mem_slave_write(uint64_t offset, uint64_t size, uint64_t *data);

    void handle_irq(uint64_t vector) override;
};