#ifndef VIDEOSTREAMER_H_
#define VIDEOSTREAMER_H_

#include <stdint.h>
#include "lwip/tcp.h"
#include "xil_cache.h"

namespace digilent {

class VideoStreamer {
public:
    VideoStreamer() :
        pcb_(nullptr),
        client_pcb_(nullptr),
        frame_counter_(0),
        is_connected_(false) {}

    // 初始化网络
    bool init(uint16_t port) {
        // 创建TCP监听PCB
        pcb_ = tcp_new();
        if (!pcb_) {
            xil_printf("Error creating PCB\r\n");
            return false;
        }

        err_t err = tcp_bind(pcb_, IP_ADDR_ANY, port);
        if (err != ERR_OK) {
            xil_printf("Bind error: %d\r\n", err);
            return false;
        }

        pcb_ = tcp_listen(pcb_);
        tcp_accept(pcb_, accept_callback);
        tcp_arg(pcb_, this);

        xil_printf("Video server listening on port %d\r\n", port);
        return true;
    }

    // 发送帧（带降采样）
    bool send_frame_downsampled(uint8_t* frame_data,
                                uint16_t src_width, uint16_t src_height,
                                uint16_t dst_width, uint16_t dst_height,
                                uint16_t frame_skip) {
        if (!is_connected_) return false;

        // 帧率控制
        frame_counter_++;
        if (frame_counter_ % frame_skip != 0) {
            return true; // 跳过此帧
        }

        // 刷新缓存
        Xil_DCacheInvalidateRange((UINTPTR)frame_data,
                                  src_width * src_height * 3);

        // 发送帧头（包含宽度、高度信息）
        uint32_t header[3];
        header[0] = 0xFFAABBCC; // 魔数，用于同步
        header[1] = dst_width;
        header[2] = dst_height;

        err_t err = tcp_write(client_pcb_, header, sizeof(header), TCP_WRITE_FLAG_COPY);
        if (err != ERR_OK) {
            xil_printf("Header write error: %d\r\n", err);
            return false;
        }

        // 降采样并发送
        uint32_t bytes_per_pixel = 3;
        uint32_t buffer_size = dst_width * bytes_per_pixel;
        uint8_t* line_buffer = new uint8_t[buffer_size];

        for (uint16_t y = 0; y < dst_height; y++) {
            // 计算源图像对应的行
            uint16_t src_y = (y * src_height) / dst_height;

            for (uint16_t x = 0; x < dst_width; x++) {
                // 计算源图像对应的列
                uint16_t src_x = (x * src_width) / dst_width;
                uint32_t src_offset = (src_y * src_width + src_x) * bytes_per_pixel;
                uint32_t dst_offset = x * bytes_per_pixel;

                // 复制像素
                line_buffer[dst_offset] = frame_data[src_offset];
                line_buffer[dst_offset + 1] = frame_data[src_offset + 1];
                line_buffer[dst_offset + 2] = frame_data[src_offset + 2];
            }

            // 发送一行数据
            err = tcp_write(client_pcb_, line_buffer, buffer_size, TCP_WRITE_FLAG_COPY);
            if (err != ERR_OK) {
                xil_printf("Line write error: %d\r\n", err);
                delete[] line_buffer;
                return false;
            }
        }

        delete[] line_buffer;

        // 刷新发送缓冲区
        tcp_output(client_pcb_);

        return true;
    }

    // 简单的JPEG头（用于更高压缩比）
    bool send_frame_jpeg_simple(uint8_t* frame_data,
                               uint16_t width, uint16_t height,
                               uint16_t frame_skip) {
        // 这里可以集成简单的JPEG编码器
        // 或者使用硬件JPEG编码器（如果FPGA中有）
        // 暂时使用降采样方案
        return send_frame_downsampled(frame_data, width, height,
                                     width/2, height/2, frame_skip);
    }

    bool is_connected() const { return is_connected_; }

private:
    // TCP接受回调
    static err_t accept_callback(void* arg, struct tcp_pcb* newpcb, err_t err) {
        VideoStreamer* streamer = static_cast<VideoStreamer*>(arg);

        if (err != ERR_OK || newpcb == nullptr) {
            return ERR_VAL;
        }

        xil_printf("Client connected\r\n");
        streamer->client_pcb_ = newpcb;
        streamer->is_connected_ = true;

        tcp_arg(newpcb, streamer);
        tcp_recv(newpcb, recv_callback);
        tcp_err(newpcb, error_callback);

        return ERR_OK;
    }

    // TCP接收回调（处理客户端命令）
    static err_t recv_callback(void* arg, struct tcp_pcb* tpcb,
                              struct pbuf* p, err_t err) {
        if (p == nullptr) {
            // 连接关闭
            VideoStreamer* streamer = static_cast<VideoStreamer*>(arg);
            streamer->is_connected_ = false;
            tcp_close(tpcb);
            xil_printf("Client disconnected\r\n");
            return ERR_OK;
        }

        tcp_recved(tpcb, p->tot_len);
        pbuf_free(p);
        return ERR_OK;
    }

    // 错误回调
    static void error_callback(void* arg, err_t err) {
        VideoStreamer* streamer = static_cast<VideoStreamer*>(arg);
        streamer->is_connected_ = false;
        xil_printf("Connection error: %d\r\n", err);
    }

private:
    struct tcp_pcb* pcb_;
    struct tcp_pcb* client_pcb_;
    uint32_t frame_counter_;
    bool is_connected_;
};

} // namespace digilent

#endif /* VIDEOSTREAMER_H_ */
