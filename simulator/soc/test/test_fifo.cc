
#include <iostream>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
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

int main() {
    const char* fifo_path_req = "./fifo/qemu_to_soc_req";
    const char* fifo_path_resp = "./fifo/qemu_to_soc_resp";
    
    // 创建命名管道（如果不存在）
    mkfifo(fifo_path_req, 0666);
    mkfifo(fifo_path_resp, 0666);
    
    std::cout << "等待读取进程打开管道..." << std::endl;
    int req_fd = open(fifo_path_req, O_WRONLY); // 阻塞直到有读取端

    int resp_fd = open(fifo_path_resp, O_RDONLY);
    
    exPktCmd cmd;
    cmd.type = EX_PKT_WR;
    cmd.length = 4; // 假设数据长度为4字节
    cmd.addr = 0x1000; // 假设地址为0x100
    cmd.data = 0x55aaaa55; // 假设要写入的数据

    exPktCmd cmd_resp;

    while (1) {
        cmd.data++;
        std::cout << "发送命令: addr = " << std::hex << cmd.addr 
                  << ", data = " << cmd.data << std::endl;
        write(req_fd, &cmd, sizeof(cmd));
        cmd.addr += 4;
        read(resp_fd, &cmd_resp, sizeof(cmd_resp));
        std::cout << "收到响应: addr = " << std::hex << cmd_resp.addr 
                  << ", data = " << cmd_resp.data << std::endl;
        sleep(1); // 每秒写入一次
    }
    
    close(req_fd);
    std::cout << "数据已写入管道" << std::endl;
    
    return 0;
}
