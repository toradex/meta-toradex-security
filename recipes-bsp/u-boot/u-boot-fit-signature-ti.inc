do_deploy:prepend() {
    # when the FIT image is generated and signed, the public key is included
    # in u-boot.dtb, and the U-boot image needs to be regenerated so the new
    # DTB is used
    eval $(cat include/config/auto.conf | grep ^CONFIG_DEFAULT_DEVICE_TREE)
    install -m 0644 u-boot.dtb arch/arm/dts/${CONFIG_DEFAULT_DEVICE_TREE}.dtb
    oe_runmake ${UBOOT_MAKE_TARGET}
    install -m 0644 ${UBOOT_BINARY} u-boot-${UBOOT_CONFIG}.img
}
