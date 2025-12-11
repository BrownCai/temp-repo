#include "udp_stream.h"
#include "lwip/pbuf.h"
#include <string.h>

#include "xparameters.h"
#include "../platform/platform.h"

// 帧头结构
struct FrameHeader {
    uint32_t frame_id;
    uint32_t packet_id;
    uint32_t total_packets;
    uint16_t packet_size;
    uint16_t width;
    uint16_t height;
};

UDPStreamer::UDPStreamer(const char* target_ip, uint16_t target_port)
    : target_port_(target_port), frame_counter_(0) {
    ipaddr_aton(target_ip, &target_addr_);
}

void UDPStreamer::init() {
    pcb_ = udp_new();
    if (pcb_ == NULL) {
//        xil_printf("UDP PCB create failed\r\n");
        return;
    }
    udp_bind(pcb_, IP_ADDR_ANY, LOCAL_PORT);
}

void UDPStreamer::sendFrame(uint8_t* frame_buffer, uint32_t frame_size) {
    uint32_t total_packets = (frame_size + UDP_PACKET_SIZE - 1) / UDP_PACKET_SIZE;

    for (uint32_t i = 0; i < total_packets; i++) {
        uint32_t offset = i * UDP_PACKET_SIZE;
        uint16_t packet_data_size = (offset + UDP_PACKET_SIZE > frame_size)
                                     ? (frame_size - offset)
                                     : UDP_PACKET_SIZE;

        // 创建pbuf
        struct pbuf* p = pbuf_alloc(PBUF_TRANSPORT,
                                    sizeof(FrameHeader) + packet_data_size,
                                    PBUF_RAM);
        if (p == NULL) continue;

        // 填充帧头
        FrameHeader* header = (FrameHeader*)p->payload;
        header->frame_id = frame_counter_;
        header->packet_id = i;
        header->total_packets = total_packets;
        header->packet_size = packet_data_size;
        header->width = FRAME_WIDTH;
        header->height = FRAME_HEIGHT;

        // 拷贝图像数据
        memcpy((uint8_t*)p->payload + sizeof(FrameHeader),
               frame_buffer + offset,
               packet_data_size);

        // 发送
        udp_sendto(pcb_, p, &target_addr_, target_port_);
        pbuf_free(p);
    }

    frame_counter_++;
}

UDPStreamer::~UDPStreamer() {
    if (pcb_) udp_remove(pcb_);
}
