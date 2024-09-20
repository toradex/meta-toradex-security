TDX_UBOOT_HARDENING_PATCHES_DOWNSTREAM = "\
    file://0001-toradex-common-add-command-whitelisting-modules.patch \
    file://0002-toradex-dts-add-fragment-file-to-configure-secure-bo.patch \
    file://0003-toradex-integrate-command-whitelisting-downstream.patch \
    file://0004-toradex-integrate-bootm-protection-downstream.patch \
    file://0005-toradex-integrate-CLI-access-protection-downstream.patch \
    file://0006-toradex-add-implementation-of-bootargs-protection.patch \
    file://0007-toradex-integrate-bootargs-protection-downstream.patch \
"

TDX_UBOOT_HARDENING_PATCHES_UPSTREAM = "\
    file://0001-toradex-common-add-command-whitelisting-modules.patch \
    file://0002-toradex-dts-add-fragment-file-to-configure-secure-bo.patch \
    file://0003-toradex-integrate-command-whitelisting-upstream.patch \
    file://0004-toradex-integrate-bootm-protection-upstream.patch \
    file://0005-toradex-integrate-CLI-access-protection-upstream.patch \
    file://0006-toradex-add-implementation-of-bootargs-protection.patch \
    file://0007-toradex-integrate-bootargs-protection-upstream.patch \
"

TDX_UBOOT_HARDENING_PATCHES = "${TDX_UBOOT_HARDENING_PATCHES_UPSTREAM}"
TDX_UBOOT_HARDENING_PATCHES:apalis-imx8 = "${TDX_UBOOT_HARDENING_PATCHES_DOWNSTREAM}"
TDX_UBOOT_HARDENING_PATCHES:colibri-imx8x = "${TDX_UBOOT_HARDENING_PATCHES_DOWNSTREAM}"

SRC_URI:append = "\
    file://u-boot-harden.cfg \
    ${TDX_UBOOT_HARDENING_PATCHES} \
"

do_compile:prepend() {
    if [ "${TDX_IMX_HAB_ENABLE}" = "0" ] && [ "${TDX_UBOOT_HARDENING_ENABLE}" = "1" ]; then
        bbfatal 'The combination TDX_IMX_HAB_ENABLE = "0" and TDX_UBOOT_HARDENING_ENABLE = "1" is not allowed: the whitelisting feature (part of the hardening) currently relies on HAB/AHAB.'
    fi
}
