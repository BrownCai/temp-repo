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
