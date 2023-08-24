SRC_URI:append = "\
    file://fit-signature.cfg \
"

do_compile:append() {
    # when u-boot.imx is the U-Boot target, u-boot.dtb is not generated,
    # and since u-boot.dtb is required to save the public key used to
    # check the FIT image signature, let's create it
    if [ "${UBOOT_MAKE_TARGET}" = "u-boot.imx" ]; then
        ln -s ${B}/dts/dt.dtb ${B}/u-boot.dtb
    fi
}

deploy_dtb() {
    # update U-Boot image with the one that has the signature node
    # this is required when the BSP is using UBOOT_CONFIG
    if [ -n "${UBOOT_CONFIG}" ]; then
        i=0
        j=0
        for config in ${UBOOT_MACHINE}; do
            i=$(expr $i + 1);
            for type in ${UBOOT_CONFIG}; do
                j=$(expr $j + 1);
                if [ $j -eq $i ]; then
                    cp ${DEPLOYDIR}/${UBOOT_IMAGE} ${B}/${config}/u-boot-$type.${UBOOT_SUFFIX}
                fi
            done
        done
    fi

    # if u-boot.imx is the U-Boot target, rebuild the U-Boot image with the new DTB
    if [ "${UBOOT_MAKE_TARGET}" = "u-boot.imx" ]; then
        cd ${B}
        oe_runmake EXT_DTB=${DEPLOY_DIR_IMAGE}/${UBOOT_DTB_IMAGE} ${UBOOT_MAKE_TARGET}
        install ${UBOOT_BINARY} ${DEPLOYDIR}/${UBOOT_IMAGE}
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

do_deploy:prepend() {
    deploy_dtb
}
