#include "cosim_bridge.hh"

void cosim_bridge::mem_slave_read(uint64_t offset, uint64_t size, uint64_t *data)
{
    exPktCmd cmd;
    cmd.addr = offset;
    cmd.length = size;
    cmd.type = EX_PKT_RD;

    ssize_t ret = write(tx_fd_req, &cmd, sizeof(cmd));
    if (ret < 0) {
        std::cerr << "Error writing to rx_fd_req: " << strerror(errno) << std::endl;
        return; // Handle error appropriately
    } else if (ret != sizeof(cmd)) {
        std::cerr << "Partial write to rx_fd_req." << std::endl;
        return; // Handle partial write appropriately
    }

    ret = read(tx_fd_resp, &cmd, sizeof(cmd));
    *data = cmd.data;
}

void cosim_bridge::mem_slave_write(uint64_t offset, uint64_t size, uint64_t *data)
{
    exPktCmd cmd;
    cmd.addr = offset;
    cmd.length = size;
    cmd.type = EX_PKT_WR;

    ssize_t ret = write(tx_fd_req, &cmd, sizeof(cmd));
    if (ret < 0) {
        std::cerr << "Error writing to rx_fd_req: " << strerror(errno) << std::endl;
        return; // Handle error appropriately
    } else if (ret != sizeof(cmd)) {
        std::cerr << "Partial write to rx_fd_req." << std::endl;
        return; // Handle partial write appropriately
    }

    ret = read(tx_fd_resp, &cmd, sizeof(cmd));
}

void cosim_bridge::handle_irq(uint64_t vector)
{
    std::cout << "handle irq: vector:" << std::hex << vector << std::endl;
}

void cosim_bridge::fifo_recv_func()
{
    rx_fd_req = open(rx_fd_req_path, O_RDONLY, 0666);
    if (rx_fd_req < 0) {
        std::cerr << "Error opening rx_fd_req: " << rx_fd_req_path << std::endl;
        //throw std::runtime_error("Failed to open rx_fd_req");
    }
    fcntl(rx_fd_req, F_SETFL, 0);

    rx_fd_resp = open(rx_fd_resp_path, O_WRONLY, 0666);
    if (rx_fd_resp < 0) {
        std::cerr << "Error opening rx_fd_resp: " << rx_fd_resp_path << std::endl;
        //throw std::runtime_error("Failed to open rx_fd_resp");
    }

    std::cout << "cosim_bridge initialized with rx_fd_req: " << rx_fd_req
                << ", rx_fd_resp: " << rx_fd_resp
                << std::endl;

    while(1) {
        exPktCmd cmd;
        ssize_t ret = read(rx_fd_req, &cmd, sizeof(cmd));
        if (ret < 0) {
            std::cerr << "Error reading from rx_fd_req: " << strerror(errno) << std::endl;
            continue; // Handle error appropriately
        } else if (ret == 0) {
            std::cout << "End of file reached on rx_fd_req." << std::endl;
            break; // Exit loop on EOF
        } else {
            // Process the command
            std::cout << "Received command: addr=0x" << std::hex << cmd.addr 
                    << ", data=0x" << cmd.data << std::endl;
            if (cmd.type == EX_PKT_RD) {
                uint64_t data = 0;
                mem_master_read(cmd.addr, cmd.length, &data);
                cmd.data = data; // Update cmd.data with the read value
                // Write response back
                ssize_t write_ret = write(rx_fd_resp, &cmd, sizeof(cmd));
                if (write_ret < 0) {
                    std::cerr << "Error writing to rx_fd_resp: " << strerror(errno) << std::endl;
                } else if (write_ret != sizeof(cmd)) {
                    std::cerr << "Partial write to rx_fd_resp." << std::endl;
                }
            } else if (cmd.type == EX_PKT_WR) {
                mem_master_write(cmd.addr, cmd.length, &cmd.data);
                cmd.type = EX_PKT_RESP_FLAG; // Set response flag
                ssize_t write_ret = write(rx_fd_resp, &cmd, sizeof(cmd));
                if (write_ret < 0) {
                    std::cerr << "Error writing to rx_fd_resp: " << strerror(errno) << std::endl;
                } else if (write_ret != sizeof(cmd)) {
                    std::cerr << "Partial write to rx_fd_resp." << std::endl;
                }
            } else if (cmd.type == EX_PKT_IRQ) {
                handle_irq(cmd.data); // Assuming cmd.data contains the vector
            } else {
                std::cerr << "Unknown command type: " << cmd.type << std::endl;
            }
        }
    }
}

void cosim_bridge::cosim_start_polling_remote()
{
    tx_fd_req = open(tx_fd_req_path, O_WRONLY, 0666);
    if (tx_fd_req < 0) {
        std::cerr << "Error opening tx_fd_req: " << tx_fd_req_path << std::endl;
        //throw std::runtime_error("Failed to open tx_fd_req");
    }
    tx_fd_resp = open(tx_fd_resp_path, O_RDONLY, 0666);
    if (tx_fd_resp < 0) {
        std::cerr << "Error opening tx_fd_resp: " << tx_fd_resp_path << std::endl;
        //throw std::runtime_error("Failed to open tx_fd_resp");
    }
    auto bindfunc = std::bind(&cosim_bridge::fifo_recv_func, this);
    std::thread t(bindfunc);
    t.join(); // Detach the thread to run independently
}