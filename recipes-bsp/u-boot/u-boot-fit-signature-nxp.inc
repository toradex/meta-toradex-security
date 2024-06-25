do_compile:append() {
    # when u-boot.imx is the U-Boot target, u-boot.dtb is not generated,
    # and since u-boot.dtb is required to save the public key used to
    # check the FIT image signature, let's create it
    if [ "${UBOOT_MAKE_TARGET}" = "u-boot.imx" ]; then
        ln -s ${B}/dts/dt.dtb ${B}/u-boot.dtb
    fi
}

common_imx6() {
    # If HAB is enabled the re-built U-Boot and SPL must be signed
    if [ "${TDX_IMX_HAB_ENABLE}" = "1" ]; then
        sign_habv4 "IMX6"
        if [ -n "${UBOOT_CONFIG}" ]; then
            i=0
            j=0
            for config in ${UBOOT_MACHINE}; do
                i=$(expr $i + 1);
                for type in ${UBOOT_CONFIG}; do
                    j=$(expr $j + 1);
                    if [ $j -eq $i ]; then
                        install ${B}/${config}/${UBOOT_BINARY} ${DEPLOYDIR}/${UBOOT_IMAGE}
                    fi
                done
            done
        fi
    fi
}

do_deploy:prepend:mx6ull-generic-bsp() {
    # If HAB is enabled the re-built U-Boot image must be signed
    if [ "${TDX_IMX_HAB_ENABLE}" = "1" ]; then
        sign_habv4 "IMX6ULL"
	install ${B}/${UBOOT_BINARY} ${DEPLOYDIR}/${UBOOT_IMAGE}
    fi
}

do_deploy:prepend:mx7-generic-bsp() {
    # If HAB is enabled the re-built U-Boot image must be signed
    if [ "${TDX_IMX_HAB_ENABLE}" = "1" ]; then
        sign_habv4 "IMX7"
        install ${B}/${UBOOT_BINARY} ${DEPLOYDIR}/${UBOOT_IMAGE}
    fi
}

do_deploy:prepend:mx6q-generic-bsp() {
    common_imx6
}

do_deploy:prepend:mx6dl-generic-bsp() {
    common_imx6
}

