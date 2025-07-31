SRC_URI:append = "\
    file://optee.cfg \
    ${@oe.utils.conditional('TDX_OPTEE_FTPM', '1', 'file://optee-ftpm.cfg', '', d)} \
"

# Required for iMX95 Verdin EVK when using the NXP BSP layer (meta-imx-bsp),
# as its kernel recipe uses a custom kernel config fragments implementation
DELTA_KERNEL_DEFCONFIG:imx95-19x19-verdin += "\
    optee.cfg \
    ${@oe.utils.conditional('TDX_OPTEE_FTPM', '1', 'optee-ftpm.cfg', '', d)} \
"

# add OP-TEE image and OP-TEE configuration nodes to FIT image
require ${@ 'linux-optee-fit.inc' if d.getVar('TDX_OPTEE_ADD_FIT_NODES') == '1' else ''}
