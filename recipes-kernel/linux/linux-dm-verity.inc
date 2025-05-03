SRC_URI += " \
    file://dm-verity.cfg \
"

# This is required for iMX95 Verdin EVK, because it uses
# the kernel recipe from NXP BSP, which has its own kernel
# config fragments implementation
DELTA_KERNEL_DEFCONFIG:imx95-19x19-verdin += "dm-verity.cfg"
