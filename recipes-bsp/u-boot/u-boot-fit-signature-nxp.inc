concat_dtb_pre_common() {
    if [ "${UBOOT_MAKE_TARGET}" != "u-boot.imx" ]; then
        bbfatal "Don't know how to handle a make target other than u-boot.imx"
    fi

    # When the make target is "u-boot.imx" the u-boot.dtb file is not generated;
    # do it now right before adding the FIT signing keys to the DTB (to be
    # performed by concat_dtb()).
    bbdebug 1 "Generating u-boot.dtb binary"
    oe_runmake ${UBOOT_DTB_BINARY}
}

concat_dtb_post_common() {
    if [ "${UBOOT_MAKE_TARGET}" != "u-boot.imx" ]; then
        bbfatal "Don't know how to handle a make target other than u-boot.imx"
    fi

    # Since concat_dtb() doesn't know how to handle a u-boot.imx file it will
    # wrongly generate it by simply concatenating it with the u-boot.dtb binary;
    # fix this by regenerating u-boot.imx now with the signed u-boot-dtb.
    rm -f "${UBOOT_MAKE_TARGET}"
    oe_runmake EXT_DTB="${UBOOT_DTB_SIGNED}" ${UBOOT_MAKE_TARGET}
}

concat_dtb:prepend:mx6ull-generic-bsp() {
    concat_dtb_pre_common "$@"
}

concat_dtb:append:mx6ull-generic-bsp() {
    concat_dtb_post_common "$@"
}

concat_dtb:prepend:mx7-generic-bsp() {
    concat_dtb_pre_common "$@"
}

concat_dtb:append:mx7-generic-bsp() {
    concat_dtb_post_common "$@"
}
