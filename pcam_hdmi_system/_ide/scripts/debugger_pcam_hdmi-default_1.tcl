# Usage with Vitis IDE:
# In Vitis IDE create a Single Application Debug launch configuration,
# change the debug type to 'Attach to running target' and provide this 
# tcl script in 'Execute Script' option.
# Path of this script: D:\EmbeddedProjects\Xilinx\zybo-z7_pcam_5c\vitis\pcam_hdmi_system\_ide\scripts\debugger_pcam_hdmi-default_1.tcl
# 
# 
# Usage with xsct:
# To debug using xsct, launch xsct and run below command
# source D:\EmbeddedProjects\Xilinx\zybo-z7_pcam_5c\vitis\pcam_hdmi_system\_ide\scripts\debugger_pcam_hdmi-default_1.tcl
# 
connect -url tcp:127.0.0.1:3121
targets -set -nocase -filter {name =~"APU*"}
rst -system
after 3000
targets -set -filter {jtag_cable_name =~ "Digilent Zybo Z7 210351A6B180A" && level==0 && jtag_device_ctx=="jsn-Zybo Z7-210351A6B180A-13722093-0"}
fpga -file D:/EmbeddedProjects/Xilinx/zybo-z7_pcam_5c/vitis/pcam_hdmi/_ide/bitstream/system_wrapper.bit
targets -set -nocase -filter {name =~"APU*"}
loadhw -hw D:/EmbeddedProjects/Xilinx/zybo-z7_pcam_5c/vitis/hw_pcam/export/hw_pcam/hw/system_wrapper.xsa -mem-ranges [list {0x40000000 0xbfffffff}] -regs
configparams force-mem-access 1
targets -set -nocase -filter {name =~"APU*"}
source D:/EmbeddedProjects/Xilinx/zybo-z7_pcam_5c/vitis/pcam_hdmi/_ide/psinit/ps7_init.tcl
ps7_init
ps7_post_config
targets -set -nocase -filter {name =~ "*A9*#0"}
dow D:/EmbeddedProjects/Xilinx/zybo-z7_pcam_5c/vitis/pcam_hdmi/Debug/pcam_hdmi.elf
configparams force-mem-access 0
targets -set -nocase -filter {name =~ "*A9*#0"}
con
