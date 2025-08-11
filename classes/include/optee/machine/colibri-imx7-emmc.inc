# extension to append to optee binary
OPTEE_BIN_EXT = "bin"

# OP-TEE load address
TDX_OPTEE_ADDR_LOAD = "0x8c000000"

# OP-TEE reserved memory
TDX_OPTEE_ADDR_SIZE ?= "0x1000000"

# add OP-TEE nodes to kernel device tree
require classes/include/optee/linux-dt-overlay.inc

# additional configuration if booting from FIT image
require ${@oe.utils.conditional('KERNEL_IMAGETYPE', 'fitImage', 'classes/include/optee/boot-from-fit.inc', '', d)}
