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

TDX_IMX8M_FIT_ARTIFACTS = "\
    file://imx8m_copy_signature_node_to_fdts.sh \
"
SRC_URI:append:mx8m-generic-bsp = "${TDX_IMX8M_FIT_ARTIFACTS}"

concat_dtb:append:mx8m-generic-bsp() {
    # The base concat_dtb() doesn't know how to handle a flash.bin and it will
    # not touch it. This means here we'll need to regenerate flash.bin with an
    # updated DTB using binman.
    bbdebug 1 "Re-generating flash.bin with an updated DTB"

    # Copy signature from u-boot.dtb-signed to the DTs listed in CONFIG_OF_LIST.
    ${WORKDIR}/imx8m_copy_signature_node_to_fdts.sh "${UBOOT_DTB_SIGNED}"

    # Execute make again to generate flash.bin. We could execute "make" passing
    # EXT_DTB="${UBOOT_DTB_SIGNED}" but this wouldn't help (or hurt) because the
    # extra signature node presente in the signed DTB is not used by binman;
    # binman uses the DTs listed in CONFIG_OF_LIST directly and that's why we
    # updated them in the previous steps.
    oe_runmake ${UBOOT_MAKE_TARGET}
}
