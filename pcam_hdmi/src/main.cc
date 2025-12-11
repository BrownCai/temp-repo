#include "xparameters.h"

#include "platform/platform.h"
#include "ov5647/OV5647.h"
#include "ov5647/ScuGicInterruptController.h"
#include "ov5647/PS_GPIO.h"
#include "ov5647/AXI_VDMA.h"
#include "ov5647/PS_IIC.h"

#include "MIPI_D_PHY_RX.h"
#include "MIPI_CSI_2_RX.h"

#include "hdmi/VideoOutput.h"

// lwIP 2.2.0 头文件
#include "lwip/ip_addr.h"
#include "lwip/init.h"
#include "lwip/udp.h"
#include "lwip/pbuf.h"
#include "lwip/inet.h"
#include "netif/etharp.h"
#include "xil_cache.h"

// PS端以太网驱动
#include "xemacps.h"
#include "netif/xadapter.h"

#define IRPT_CTL_DEVID 		XPAR_PS7_SCUGIC_0_DEVICE_ID
#define GPIO_DEVID			XPAR_PS7_GPIO_0_DEVICE_ID
#define GPIO_IRPT_ID			XPAR_PS7_GPIO_0_INTR
#define CAM_I2C_DEVID		XPAR_PS7_I2C_0_DEVICE_ID
#define CAM_I2C_IRPT_ID		XPAR_PS7_I2C_0_INTR
#define VDMA_DEVID			XPAR_AXIVDMA_0_DEVICE_ID
#define VDMA_MM2S_IRPT_ID	XPAR_FABRIC_AXI_VDMA_0_MM2S_INTROUT_INTR
#define VDMA_S2MM_IRPT_ID	XPAR_FABRIC_AXI_VDMA_0_S2MM_INTROUT_INTR
#define CAM_I2C_SCLK_RATE	100000

#define DDR_BASE_ADDR		XPAR_DDR_MEM_BASEADDR
#define MEM_BASE_ADDR		(DDR_BASE_ADDR + 0x0A000000)

#define GAMMA_BASE_ADDR     XPAR_AXI_GAMMACORRECTION_1_BASEADDR

#define UDP_PORT            8080
#define CLIENT_PORT         9090
#define MAX_UDP_PACKET      1400

//// 使用PS端的GigE控制器
//#ifndef XPAR_XEMACPS_0_DEVICE_ID
//    #define XPAR_XEMACPS_0_DEVICE_ID XPAR_PS7_ETHERNET_0_DEVICE_ID
//#endif

using namespace digilent;

// 网络配置
static struct netif server_netif;
static struct udp_pcb* udp_pcb_inst = NULL;
static ip_addr_t client_ip;
static u16_t client_port = 0;
static volatile bool client_registered = false;
static uint32_t frame_counter = 0;

// 声明外部lwIP网络接口初始化函数
extern "C" {
    err_t xemacpsif_init(struct netif *netif);
    int xemacpsif_input(struct netif *netif);
}

// UDP接收回调
void udp_recv_callback(void *arg, struct udp_pcb *pcb, struct pbuf *p,
                       const ip_addr_t *addr, u16_t port) {
    if (p != NULL) {
        if (!client_registered) {
            ip_addr_copy(client_ip, *addr);
            client_port = port;
            client_registered = true;
            xil_printf("Client registered: %s:%d\r\n",
                      ipaddr_ntoa(addr), port);
        }
        pbuf_free(p);
    }
}

// 直接初始化PS端GigE网络接口
bool init_network() {
    err_t err;
    ip_addr_t ipaddr, netmask, gw;

    xil_printf("\r\n=== Initializing PS GigE Network ===\r\n");

    // 配置静态IP
    IP4_ADDR(&ipaddr,  192, 168, 1, 10);
    IP4_ADDR(&netmask, 255, 255, 255, 0);
    IP4_ADDR(&gw,      192, 168, 1, 1);

    xil_printf("Device IP: 192.168.1.10\r\n");
    xil_printf("Netmask: 255.255.255.0\r\n");
    xil_printf("Gateway: 192.168.1.1\r\n");

    // 初始化lwIP
    lwip_init();
    xil_printf("lwIP 2.2.0 initialized\r\n");

    // 设置MAC地址
    unsigned char mac_address[6] = {0x00, 0x0a, 0x35, 0x00, 0x01, 0x02};

    // 直接使用netif_add添加PS GigE接口
    struct netif *netif_ptr = netif_add(&server_netif,
                                        ip_2_ip4(&ipaddr),
                                        ip_2_ip4(&netmask),
                                        ip_2_ip4(&gw),
                                        (void*)XPAR_XEMACPS_0_DEVICE_ID,
                                        xemacpsif_init,
                                        ethernet_input);

    if (netif_ptr == NULL) {
        xil_printf("ERROR: netif_add failed\r\n");
        return false;
    }

    xil_printf("Network interface added (PS GigE)\r\n");

    // 设置MAC地址
    server_netif.hwaddr_len = 6;
    memcpy(server_netif.hwaddr, mac_address, 6);

    // 设置为默认接口并启动
    netif_set_default(&server_netif);
    netif_set_up(&server_netif);

    if (!netif_is_up(&server_netif)) {
        xil_printf("ERROR: Network interface is DOWN\r\n");
        return false;
    }

    xil_printf("Network interface is UP\r\n");
    xil_printf("MAC: %02x:%02x:%02x:%02x:%02x:%02x\r\n",
               mac_address[0], mac_address[1], mac_address[2],
               mac_address[3], mac_address[4], mac_address[5]);

    // 创建UDP PCB
    udp_pcb_inst = udp_new();
    if (udp_pcb_inst == NULL) {
        xil_printf("ERROR: Cannot create UDP PCB\r\n");
        return false;
    }
    xil_printf("UDP PCB created\r\n");

    // 绑定到本地端口
    err = udp_bind(udp_pcb_inst, IP_ADDR_ANY, UDP_PORT);
    if (err != ERR_OK) {
        xil_printf("ERROR: UDP bind failed: %d\r\n", err);
        return false;
    }
    xil_printf("UDP bound to port %d\r\n", UDP_PORT);

    // 设置接收回调
    udp_recv(udp_pcb_inst, udp_recv_callback, NULL);

    xil_printf("=== Network Ready ===\r\n");
    xil_printf("Send 'HELLO' to 192.168.1.10:%d to register\r\n\r\n", UDP_PORT);

    return true;
}

// 发送UDP包
bool send_udp_packet(const void* data, u16_t len) {
    if (!client_registered || udp_pcb_inst == NULL) {
        return false;
    }

    struct pbuf *p = pbuf_alloc(PBUF_TRANSPORT, len, PBUF_RAM);
    if (p == NULL) {
        return false;
    }

    memcpy(p->payload, data, len);

    err_t err = udp_sendto(udp_pcb_inst, p, &client_ip, client_port);

    pbuf_free(p);

    return (err == ERR_OK);
}

// 发送帧（UDP分包传输）
bool send_frame_udp(uint8_t* frame_data, uint16_t src_w, uint16_t src_h,
                    uint16_t dst_w, uint16_t dst_h) {
    if (!client_registered) {
        return false;
    }

    // 刷新缓存
    Xil_DCacheInvalidateRange((UINTPTR)frame_data, src_w * src_h * 3);

    // 帧头
    struct {
        uint32_t magic;
        uint32_t frame_id;
        uint32_t width;
        uint32_t height;
        uint32_t total_packets;
    } frame_header;

    frame_header.magic = 0xFFAABBCC;
    frame_header.frame_id = frame_counter;
    frame_header.width = dst_w;
    frame_header.height = dst_h;

    // 计算包数
    uint32_t frame_size = dst_w * dst_h * 3;
    uint32_t data_per_packet = MAX_UDP_PACKET - 8;
    uint32_t total_packets = (frame_size + data_per_packet - 1) / data_per_packet;

    frame_header.total_packets = total_packets;

    // 发送帧头
    if (!send_udp_packet(&frame_header, sizeof(frame_header))) {
        return false;
    }

    // 降采样到临时缓冲区
    static uint8_t temp_buffer[640 * 360 * 3];
    uint32_t temp_offset = 0;

    for (uint16_t y = 0; y < dst_h; y++) {
        uint16_t src_y = (y * src_h) / dst_h;

        for (uint16_t x = 0; x < dst_w; x++) {
            uint16_t src_x = (x * src_w) / dst_w;
            uint32_t src_idx = (src_y * src_w + src_x) * 3;

            temp_buffer[temp_offset++] = frame_data[src_idx];
            temp_buffer[temp_offset++] = frame_data[src_idx + 1];
            temp_buffer[temp_offset++] = frame_data[src_idx + 2];
        }
    }

    // 分包发送
    struct {
        uint32_t frame_id;
        uint32_t packet_id;
        uint8_t data[MAX_UDP_PACKET - 8];
    } data_packet;

    uint32_t offset = 0;
    for (uint32_t pkt = 0; pkt < total_packets; pkt++) {
        data_packet.frame_id = frame_counter;
        data_packet.packet_id = pkt;

        uint32_t bytes_to_send = (offset + data_per_packet > frame_size) ?
                                 (frame_size - offset) : data_per_packet;

        memcpy(data_packet.data, temp_buffer + offset, bytes_to_send);

        if (!send_udp_packet(&data_packet, 8 + bytes_to_send)) {
            return false;
        }

        offset += bytes_to_send;

        // 避免网络拥塞
        if (pkt % 10 == 0) {
            for (volatile int i = 0; i < 500; i++);
        }
    }

    return true;
}

void pipeline_mode_change(AXI_VDMA<ScuGicInterruptController>& vdma_driver, OV5647& cam, VideoOutput& vid, Resolution res, OV5647_cfg::mode_t mode)
{
	//Bring up input pipeline back-to-front
	{
		vdma_driver.resetWrite();
		MIPI_CSI_2_RX_mWriteReg(XPAR_MIPI_CSI_2_RX_0_S_AXI_LITE_BASEADDR, CR_OFFSET, (CR_RESET_MASK & ~CR_ENABLE_MASK));
		MIPI_D_PHY_RX_mWriteReg(XPAR_MIPI_D_PHY_RX_0_S_AXI_LITE_BASEADDR, CR_OFFSET, (CR_RESET_MASK & ~CR_ENABLE_MASK));
		cam.reset();
	}

	{
		vdma_driver.configureWrite(timing[static_cast<int>(res)].h_active, timing[static_cast<int>(res)].v_active);
		Xil_Out32(GAMMA_BASE_ADDR, 3); // Set Gamma correction factor to 1/1.8
		//TODO CSI-2, D-PHY config here
		cam.init();
	}

	{
		vdma_driver.enableWrite();
		MIPI_CSI_2_RX_mWriteReg(XPAR_MIPI_CSI_2_RX_0_S_AXI_LITE_BASEADDR, CR_OFFSET, CR_ENABLE_MASK);
		MIPI_D_PHY_RX_mWriteReg(XPAR_MIPI_D_PHY_RX_0_S_AXI_LITE_BASEADDR, CR_OFFSET, CR_ENABLE_MASK);
		cam.set_mode(mode);
		cam.set_awb(OV5647_cfg::awb_t::AWB_ADVANCED);
	}

	//Bring up output pipeline back-to-front
	{
		vid.reset();
		vdma_driver.resetRead();
	}

	{
		vid.configure(res);
		vdma_driver.configureRead(timing[static_cast<int>(res)].h_active, timing[static_cast<int>(res)].v_active);
	}

	{
		vid.enable();
		vdma_driver.enableRead();
	}
}

int main()
{
	init_platform();
	xil_printf("platform init done.\r\n");

	ScuGicInterruptController irpt_ctl(IRPT_CTL_DEVID);
	PS_GPIO<ScuGicInterruptController> gpio_driver(GPIO_DEVID, irpt_ctl, GPIO_IRPT_ID);
	PS_IIC<ScuGicInterruptController> iic_driver(CAM_I2C_DEVID, irpt_ctl, CAM_I2C_IRPT_ID, 100000);

	OV5647 cam(iic_driver, gpio_driver);
	AXI_VDMA<ScuGicInterruptController> vdma_driver(VDMA_DEVID, MEM_BASE_ADDR, irpt_ctl,
			VDMA_MM2S_IRPT_ID,
			VDMA_S2MM_IRPT_ID);
	VideoOutput vid(XPAR_VTC_0_DEVICE_ID, XPAR_VIDEO_DYNCLK_DEVICE_ID);

	Resolution current_res = Resolution::R1280_720_60_PP;

//	pipeline_mode_change(vdma_driver, cam, vid, Resolution::R1920_1080_60_PP, OV5647_cfg::mode_t::MODE_1080P_1920_1080_15fps);
	pipeline_mode_change(vdma_driver, cam, vid, Resolution::R1280_720_60_PP, OV5647_cfg::mode_t::MODE_720P_1280_720_60fps);


	xil_printf("Video init done.\r\n");

    // 初始化网络
    if (!init_network()) {
        xil_printf("Network initialization failed!\r\n");
    }

    // 等待视频稳定
    xil_printf("Waiting for video to stabilize...\r\n");
    for(volatile int i = 0; i < 10000000; i++);

    // 获取帧参数
	uint16_t src_width = timing[static_cast<int>(current_res)].h_active;
	uint16_t src_height = timing[static_cast<int>(current_res)].v_active;
	uint32_t frame_size = src_width * src_height * 3;
	uint32_t frame1_addr = MEM_BASE_ADDR + frame_size;
	uint8_t* frame_ptr = (uint8_t*)frame1_addr;

	uint16_t dst_width = 640;
	uint16_t dst_height = 360;

	xil_printf("\r\n=== Video Streaming Started ===\r\n");
	xil_printf("Source: %dx%d -> Target: %dx%d\r\n",
			   src_width, src_height, dst_width, dst_height);
	xil_printf("Waiting for client...\r\n\r\n");

	uint32_t sent_frames = 0;
	uint32_t loop_count = 0;

	while (1) {
        // 处理网络输入
        xemacpsif_input(&server_netif);

        // 状态输出
        if (loop_count % 10000 == 0 && !client_registered) {
            xil_printf("Waiting for client registration...\r\n");
        }

        // 发送视频帧
        if (client_registered && (frame_counter % 6 == 0)) {
            if (send_frame_udp(frame_ptr, src_width, src_height, dst_width, dst_height)) {
                sent_frames++;
                if (sent_frames % 30 == 0) {
                    xil_printf("Sent %d frames\r\n", sent_frames);
                }
            }
        }

        frame_counter++;
        loop_count++;

        // 控制循环频率
        for (volatile int i = 0; i < 3000; i++);
	}


	cleanup_platform();

	return 0;
}
