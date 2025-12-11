platform generate -domains 
platform generate
platform generate
platform clean
platform generate
platform active {hw_pcam}
domain active {standalone_domain}
bsp reload
bsp setlib -name lwip220 -ver 1.0
bsp config phy_link_speed "CONFIG_LINKSPEED1000"
bsp write
bsp reload
catch {bsp regenerate}
bsp config pbuf_pool_size "2048"
bsp write
bsp reload
catch {bsp regenerate}
platform clean
platform generate
bsp reload
bsp config mem_size "262144"
bsp config memp_n_pbuf "16"
bsp config memp_n_pbuf "64"
bsp config memp_n_sys_timeout "8"
bsp config memp_n_tcp_pcb "32"
bsp config memp_n_sys_timeout "8"
bsp config pbuf_pool_size "2048"
bsp config pbuf_pool_size "512"
bsp config tcp_snd_buf "8192"
bsp config tcp_snd_buf "32768"
bsp config tcp_synmaxrtx "4"
bsp write
bsp reload
catch {bsp regenerate}
bsp reload
bsp reload
bsp reload
bsp config lwip_tcp "false"
bsp write
bsp reload
catch {bsp regenerate}
platform clean
catch {bsp regenerate}
bsp write
bsp config lwip_tcp "true"
bsp write
bsp reload
catch {bsp regenerate}
bsp config tcp_snd_buf "8192"
bsp config tcp_mss "1460"
bsp config tcp_queue_ooseq "1"
bsp config lwip_tcp "true"
bsp write
bsp reload
catch {bsp regenerate}
platform generate
platform clean
platform generate
platform clean
platform generate
platform clean
platform generate
platform clean
platform generate
bsp reload
bsp reload
platform generate -domains 
bsp reload
bsp reload
