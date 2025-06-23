#include "ip.hh"
#include <iostream>

#include <cstdint>

#include <fcntl.h>
#include <unistd.h>

#include <thread>
#include <errno.h>
#include <string.h>
#include <functional>

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

    cosim_bridge(baseBus *bus, uint64_t id,
                 uint64_t base_address, uint64_t size,
                 uint64_t irq_vec_start, uint64_t irq_vector_cnt,
                 char *rx_fd_req_path, char *rx_fd_resp_path,
                 char *tx_fd_req_path, char *tx_fd_resp_path)
        : base_ip(bus, id, IP_TYPE_PERIPHERAL, base_address, size, irq_vec_start, irq_vector_cnt) {
        this->rx_fd_req_path = rx_fd_req_path;
        this->rx_fd_resp_path = rx_fd_resp_path;
        this->tx_fd_req_path = tx_fd_req_path;
        this->tx_fd_resp_path = tx_fd_resp_path;
    }

    ~cosim_bridge() override {
        if (rx_fd_req >= 0) close(rx_fd_req);
        if (rx_fd_resp >= 0) close(rx_fd_resp);
        if (tx_fd_req >= 0) close(tx_fd_req);
        if (tx_fd_resp >= 0) close(tx_fd_resp);
    }

    void reset() override {
        std::cout << "reset...\n";
    }

    void mem_slave_read(uint64_t offset, uint64_t size, uint64_t *data);
    void mem_slave_write(uint64_t offset, uint64_t size, uint64_t *data);

    void handle_irq(uint64_t vector) override;

    void fifo_recv_func();
    void fifo_send_func();
    void cosim_start_polling_remote();

private:
    int rx_fd_req;
    int rx_fd_resp;
    int tx_fd_req;
    int tx_fd_resp;
    char *rx_fd_req_path;
    char *rx_fd_resp_path;
    char *tx_fd_req_path;
    char *tx_fd_resp_path;
};