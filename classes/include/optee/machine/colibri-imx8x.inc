# extension to append to optee binary
OPTEE_BIN_EXT = "bin"

# OP-TEE boot images to be deployed
OPTEE_BOOT_IMAGE:forcevariable = "tee.bin uTee-${OPTEE_BIN_EXT}"

# OP-TEE load address
TDX_OPTEE_ADDR_LOAD = "0xA0000000"

# OP-TEE reserved memory
TDX_OPTEE_ADDR_SIZE ?= "0x1000000"

#DEPLOY_OPTEE:forcevariable = "false"
#ATF_MACHINE_NAME:forcevariable = "bl31-${ATF_PLATFORM}.bin"
MACHINE_FEATURES:remove:pn-imx-boot = "optee"
#DEPLOY_OPTEE:pn-imx-boot:forcevariable = "false"

# add OP-TEE nodes to kernel device tree
require classes/include/optee/linux-dt-overlay.inc

# additional configuration if booting from FIT image
require ${@oe.utils.conditional('KERNEL_IMAGETYPE', 'fitImage', 'classes/include/optee/boot-from-fit.inc', '', d)}
