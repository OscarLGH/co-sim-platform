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