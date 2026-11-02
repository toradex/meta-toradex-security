# generate device tree overlay with OP-TEE nodes
TDX_OPTEE_DT_OVERLAY = "1"

# name of the overlay file (without extension) that will contain the OP-TEE nodes,
# generated in the kernel tree. It must not match TDX_KERNEL_DEVICETREE_PREFIX,
# or the BSP would also add it to KERNEL_DEVICETREE, listing it twice (which breaks
# the FIT build).
TDX_OPTEE_DT_OVERLAY_NAME = "${MACHINE}_optee_overlay"
KERNEL_DEVICETREE:append = " ${KERNEL_DTB_PREFIX}${TDX_OPTEE_DT_OVERLAY_NAME}.dtbo"

# load OP-TEE overlay at boot time
TEZI_EXTERNAL_KERNEL_DEVICETREE_BOOT:append = " ${TDX_OPTEE_DT_OVERLAY_NAME}.dtbo"
