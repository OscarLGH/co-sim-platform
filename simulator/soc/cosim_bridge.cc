#include "cosim_bridge.hh"

void cosim_bridge::mem_slave_read(uint64_t offset, uint64_t size, void *data)
{
    exPktCmd cmd;
    cmd.addr = offset;
    cmd.length = size;
    cmd.type = EX_PKT_RD;

    ssize_t ret = write(tx_fd_req, &cmd, sizeof(cmd));
    if (ret < 0) {
        LOG_ERROR("Error writing to tx_fd_req: %s", strerror(errno));
        return; // Handle error appropriately
    } else if (ret != sizeof(cmd)) {
        LOG_ERROR("Partial write to tx_fd_req.");
        return; // Handle partial write appropriately
    }

    ret = read(tx_fd_resp, &cmd, sizeof(cmd));
    memcpy(data, &cmd.data, size);
}

void cosim_bridge::mem_slave_write(uint64_t offset, uint64_t size, void *data)
{
    exPktCmd cmd;
    cmd.addr = offset;
    cmd.length = size;
    cmd.type = EX_PKT_WR;
    memcpy(&cmd.data, data, size);

    ssize_t ret = write(tx_fd_req, &cmd, sizeof(cmd));
    if (ret < 0) {
        LOG_ERROR("Error writing to tx_fd_req: %s", strerror(errno));
        return; // Handle error appropriately
    } else if (ret != sizeof(cmd)) {
        LOG_ERROR("Partial write to tx_fd_req.");
        return; // Handle partial write appropriately
    }

    ret = read(tx_fd_resp, &cmd, sizeof(cmd));
}

void cosim_bridge::handle_irq(uint64_t vector)
{
    LOG_DEBUG("cosim_bridge handling IRQ vector %lu", vector);
}

void cosim_bridge::fifo_recv_func()
{
    rx_fd_req = open(rx_fd_req_path, O_RDONLY, 0666);
    if (rx_fd_req < 0) {
        LOG_ERROR("Error opening rx_fd_req: %s", strerror(errno));
    }
    fcntl(rx_fd_req, F_SETFL, 0);

    rx_fd_resp = open(rx_fd_resp_path, O_WRONLY, 0666);
    if (rx_fd_resp < 0) {
        LOG_ERROR("Error opening rx_fd_resp: %s", strerror(errno));
    }

    LOG_INFO("cosim_bridge initialized with rx_fd_req: %s, rx_fd_resp: %s.",
             rx_fd_req_path, rx_fd_resp_path);
    
    while(1) {
        exPktCmd cmd;
        ssize_t ret = read(rx_fd_req, &cmd, sizeof(cmd));
        if (ret < 0) {
            LOG_ERROR("Error reading from rx_fd_req: %s", strerror(errno));
            continue; // Handle error appropriately
        } else if (ret == 0) {
            LOG_ERROR("EOF reached on rx_fd_req, exiting loop.");
            break; // Exit loop on EOF
        } else {
            // Process the command
            LOG_DEBUG("Received command: type=%d, addr=0x%lx, length=%d, data=0x%lx",
                      cmd.type, cmd.addr, cmd.length, cmd.data);
            if (cmd.type == EX_PKT_RD) {
                uint64_t data = 0;
                mem_master_read(cmd.addr, cmd.length, &data);
                cmd.data = data; // Update cmd.data with the read value
                // Write response back
                ssize_t write_ret = write(rx_fd_resp, &cmd, sizeof(cmd));
                if (write_ret < 0) {
                    LOG_ERROR("Error writing to rx_fd_resp: %s", strerror(errno));
                } else if (write_ret != sizeof(cmd)) {
                    LOG_ERROR("Partial write to rx_fd_resp.");
                }
            } else if (cmd.type == EX_PKT_WR) {
                mem_master_write(cmd.addr, cmd.length, &cmd.data);
                cmd.type = EX_PKT_RESP_FLAG; // Set response flag
                ssize_t write_ret = write(rx_fd_resp, &cmd, sizeof(cmd));
                if (write_ret < 0) {
                    LOG_ERROR("Error writing to rx_fd_resp: %s", strerror(errno));
                } else if (write_ret != sizeof(cmd)) {
                    LOG_ERROR("Partial write to rx_fd_resp.");
                }
            } else if (cmd.type == EX_PKT_IRQ) {
                handle_irq(cmd.data); // Assuming cmd.data contains the vector
            } else {
                LOG_ERROR("Unknown command type: %d", cmd.type);
            }
        }
    }
}

void cosim_bridge::cosim_start_polling_remote()
{
    LOG_DEBUG("cosim_bridge starting polling remote.");
    LOG_DEBUG("opening fifo:%s\n", tx_fd_req_path);
    tx_fd_req = open(tx_fd_req_path, O_WRONLY, 0666);
    if (tx_fd_req < 0) {
        LOG_ERROR("Error opening tx_fd_req: %s", strerror(errno));
        //throw std::runtime_error("Failed to open tx_fd_req");
    }
    LOG_DEBUG("opening fifo:%s\n", tx_fd_resp_path);
    tx_fd_resp = open(tx_fd_resp_path, O_RDONLY, 0666);
    if (tx_fd_resp < 0) {
        LOG_ERROR("Error opening tx_fd_resp: %s", strerror(errno));
        //throw std::runtime_error("Failed to open tx_fd_resp");
    }

    LOG_DEBUG("start listening...\n");
    auto bindfunc = std::bind(&cosim_bridge::fifo_recv_func, this);
    std::thread t(bindfunc);
    t.detach(); // Detach the thread to run independently
}