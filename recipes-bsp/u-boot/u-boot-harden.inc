# TODO: handle machines using upstream U-Boot (use-mainline-bsp)
SRC_URI:append:use-nxp-bsp = "\
    file://u-boot-harden.cfg \
    file://0001-toradex-common-add-command-whitelisting-modules.patch \
    file://0002-toradex-dts-add-fragment-file-to-configure-secure-bo.patch \
    file://0003-toradex-integrate-command-whitelisting-downstream.patch \
    file://0004-toradex-integrate-bootm-protection-downstream.patch \
    file://0005-toradex-integrate-CLI-access-protection-downstream.patch \
"

# TODO: handle machines using upstream U-Boot (use-mainline-bsp) - remove this block.
do_compile:prepend:use-mainline-bsp() {
    bbfatal "U-Boot hardening is not currently supported with the mainline BSP."
}

do_compile:prepend() {
    if [ "${TDX_IMX_HAB_ENABLE}" = "0" -a "${TDX_UBOOT_HARDENING_ENABLE}" = "1" ]; then
        bbfatal 'The combination TDX_IMX_HAB_ENABLE = "0" and TDX_UBOOT_HARDENING_ENABLE = "1" is not allowed: the whitelisting feature (part of the hardening) currently relies on HAB/AHAB.'
    fi
}
