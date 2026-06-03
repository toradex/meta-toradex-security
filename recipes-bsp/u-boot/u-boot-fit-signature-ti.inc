rebuild_uboot() {
    type=$1
    eval $(cat include/config/auto.conf | grep ^CONFIG_DEFAULT_DEVICE_TREE)
    install -m 0644 u-boot.dtb arch/arm/dts/${CONFIG_DEFAULT_DEVICE_TREE}.dtb
    oe_runmake ${UBOOT_MAKE_TARGET}
    if [ -n "$type" ]; then
        install -m 0644 ${UBOOT_BINARY} u-boot-${type}.img
    fi
}

do_deploy:prepend() {
    # when the FIT image is generated and signed, the public key is included
    # in u-boot.dtb, and the U-boot image needs to be regenerated so the new
    # DTB is used
    if [ -n "${UBOOT_CONFIG}" ]; then
        unset i j
        for config in ${UBOOT_MACHINE}; do
            i=$(expr $i + 1)
            for type in ${UBOOT_CONFIG}; do
                j=$(expr $j + 1)
                if [ $j -eq $i ]; then
                    cd ${B}/${config}
                    rebuild_uboot $type
                fi
            done
        done
    else
        cd ${B}
        rebuild_uboot
    fi
}
