# for SoMs that boot OP-TEE from U-Boot (SPL -> U-Boot -> OP-TEE -> Linux),
# and when not using a FIT image, we need to append additional commands to
# the KERNEL_BOOTCMD variable, so the OP-TEE firmware is properly loaded
# into memory and booted before the Linux kernel
KERNEL_BOOTCMD_OPTEE = "\
setenv tee_addr_r ${TDX_OPTEE_ADDR_LOAD}; \
load ${boot_devtype} ${boot_devnum}:${boot_part} ${tee_addr_r} uTee-bin; \
setenv kernel_addr_r ${tee_addr_r}; \
bootm"
KERNEL_BOOTCMD:mx6-generic-bsp = "${KERNEL_BOOTCMD_OPTEE}"

# DTB_PREFIX is an internal recipe variable used to append a value
# to the FIT image configuration node that will be used to boot.
# Here we add 'optee_' so the configuration node with OP-TEE is used
# for selected platforms
DTB_PREFIX_OPTEE = "optee_"
DTB_PREFIX:prepend:mx6-generic-bsp = "${DTB_PREFIX_OPTEE}"
