# 
# Usage: To re-create this platform project launch xsct with below options.
# xsct D:\temp-repo\hw_pcam\platform.tcl
# 
# OR launch xsct and run below command.
# source D:\temp-repo\hw_pcam\platform.tcl
# 
# To create the platform in a different location, modify the -out option of "platform create" command.
# -out option specifies the output directory of the platform project.

platform create -name {hw_pcam}\
-hw {D:\EmbeddedProjects\Xilinx\zybo-z7_pcam_5c\Zybo-Z7-HW\pcam_ethernet.xsa}\
-proc {ps7_cortexa9_0} -os {standalone} -out {D:/temp-repo}

platform write
bsp reload
bsp setlib -name lwip220 -ver 1.0
bsp config phy_link_speed "CONFIG_LINKSPEED1000"
bsp write
bsp reload
catch {bsp regenerate}
