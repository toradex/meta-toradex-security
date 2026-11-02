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
KERNEL_BOOTCMD:mx7-generic-bsp = "${KERNEL_BOOTCMD_OPTEE}"

# When booting a FIT image, use the configuration node with OP-TEE for the
# base device tree on selected platforms. Only configurations with a kernel
# have an OP-TEE variant, so overlay configurations keep their name, which is
# why DTB_PREFIX (also used for the overlays) is not changed.
DTB_PREFIX_OPTEE = "optee_"

amend_optee_fit_conf() {
    if ! grep -q '#conf-${DTB_PREFIX}\\\$''{fdtfile}' boot.cmd; then
        bbfatal "Amendments to bootscript must be reviewed: FIT configuration of the base device tree wasn't found."
    fi
    sed -e 's/#conf-${DTB_PREFIX}\\\$''{fdtfile}/#conf-${DTB_PREFIX_OPTEE}${DTB_PREFIX}\\\$''{fdtfile}/' \
        -i boot.cmd
}

do_compile:append:mx6-generic-bsp() {
    amend_optee_fit_conf
}

do_compile:append:mx7-generic-bsp() {
    amend_optee_fit_conf
}
