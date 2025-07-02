# generate device tree overlay with OP-TEE nodes
TDX_OPTEE_DT_OVERLAY = "1"

# load OP-TEE overlay at boot time
TEZI_EXTERNAL_KERNEL_DEVICETREE_BOOT:append = " ${MACHINE_PREFIX}_optee_overlay.dtbo"
