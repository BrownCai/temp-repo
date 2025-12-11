#ifndef UDP_STREAM_H_
#define UDP_STREAM_H_

#include "lwip/udp.h"
#include "lwip/ip_addr.h"

#include "xparameters.h"
#include "../platform/platform.h"

#define TARGET_IP "192.168.1.100"  // PC IP
#define TARGET_PORT 8888
#define LOCAL_PORT 7777

#define FRAME_WIDTH 1280
#define FRAME_HEIGHT 720
#define BYTES_PER_PIXEL 3  // RGB
#define UDP_PACKET_SIZE 1400

class UDPStreamer {
public:
    UDPStreamer(const char* target_ip, uint16_t target_port);
    void sendFrame(uint8_t* frame_buffer, uint32_t frame_size);
    void init();
    ~UDPStreamer();

private:
    struct udp_pcb* pcb_;
    ip_addr_t target_addr_;
    uint16_t target_port_;
    uint32_t frame_counter_;

    void sendPacket(uint8_t* data, uint16_t len, uint32_t offset);
};

#endif
