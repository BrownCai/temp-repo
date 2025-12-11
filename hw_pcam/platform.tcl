platform generate -domains 
platform generate
platform generate
platform generate
platform active {hw_pcam}
platform config -updatehw {D:/EmbeddedProjects/Xilinx/zybo-z7_pcam_5c/Zybo-Z7-HW/pcam_ethernet.xsa}
bsp reload
domain active {standalone_domain}
bsp reload
catch {bsp regenerate}
platform clean
platform generate
platform clean
bsp reload
domain active {zynq_fsbl}
bsp reload
domain active {standalone_domain}
bsp setlib -name lwip220 -ver 1.0
bsp config phy_link_speed "CONFIG_LINKSPEED1000"
bsp config tcp_ip_rx_checksum_offload "false"
bsp config phy_link_speed "CONFIG_LINKSPEED1000"
bsp config tcp_ip_rx_checksum_offload "false"
bsp write
bsp reload
catch {bsp regenerate}
platform generate
platform config -updatehw {D:/EmbeddedProjects/Xilinx/zybo-z7_pcam_5c/Zybo-Z7-HW/pcam_ethernet.xsa}
bsp reload
bsp config phy_link_speed "CONFIG_LINKSPEED1000"
bsp config phy_link_speed "CONFIG_LINKSPEED1000"
bsp reload
platform generate -domains 
platform config -updatehw {D:/EmbeddedProjects/Xilinx/zybo-z7_pcam_5c/Zybo-Z7-HW/pcam_ethernet.xsa}
platform clean
bsp reload
bsp reload
platform config -updatehw {D:/EmbeddedProjects/Xilinx/zybo-z7_pcam_5c/Zybo-Z7-HW/pcam_ethernet.xsa}
bsp reload
catch {bsp regenerate}
bsp config phy_link_speed "CONFIG_LINKSPEED1000"
bsp write
