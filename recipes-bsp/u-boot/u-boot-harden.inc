TDX_UBOOT_HARDENING_PATCHES_DOWNSTREAM = "\
    file://0001-toradex-common-add-command-whitelisting-modules.patch \
    file://0002-toradex-dts-add-fragment-file-to-configure-secure-bo.patch \
    file://0003-toradex-integrate-command-whitelisting-downstream.patch \
    file://0004-toradex-integrate-bootm-protection-downstream.patch \
    file://0005-toradex-integrate-CLI-access-protection-downstream.patch \
    file://0006-arm-dts-add-missing-newline-to-eof-downstream.patch \
    file://0007-toradex-add-implementation-of-bootargs-protection.patch \
    file://0008-toradex-integrate-bootargs-protection-downstream.patch \
    file://0009-toradex-show-message-if-CLI-access-allowed-common.patch \
    file://0010-toradex-show-message-if-CLI-access-allowed-downstrea.patch \
"

TDX_UBOOT_HARDENING_PATCHES_DOWNSTREAM_AM62 = "\
    file://0001-toradex-common-add-command-whitelisting-modules.patch \
    file://0002-toradex-dts-add-fragment-file-to-configure-secure-bo.patch \
    file://0003-toradex-integrate-command-whitelisting-am62.patch \
    file://0004-toradex-integrate-bootm-protection-am62.patch \
    file://0005-toradex-integrate-CLI-access-protection-am62.patch \
    file://0006-arm-dts-add-missing-newline-to-eof-am62.patch \
    file://0007-toradex-add-implementation-of-bootargs-protection.patch \
    file://0008-toradex-integrate-bootargs-protection-am62.patch \
    file://0009-toradex-show-message-if-CLI-access-allowed-common.patch \
    file://0010-toradex-show-message-if-CLI-access-allowed-am62.patch \
"

TDX_UBOOT_HARDENING_PATCHES_UPSTREAM = "\
    file://0001-toradex-common-add-command-whitelisting-modules.patch \
    file://0002-toradex-dts-add-fragment-file-to-configure-secure-bo.patch \
    file://0003-toradex-integrate-command-whitelisting-upstream.patch \
    file://0004-toradex-integrate-bootm-protection-upstream.patch \
    file://0005-toradex-integrate-CLI-access-protection-upstream.patch \
    file://0006-arm-dts-add-missing-newline-to-eof-upstream.patch \
    file://0007-toradex-add-implementation-of-bootargs-protection.patch \
    file://0008-toradex-integrate-bootargs-protection-upstream.patch \
    file://0009-toradex-show-message-if-CLI-access-allowed-common.patch \
    file://0010-toradex-show-message-if-CLI-access-allowed-upstream.patch \
"

TDX_UBOOT_HARDENING_PATCHES = "${TDX_UBOOT_HARDENING_PATCHES_UPSTREAM}"
TDX_UBOOT_HARDENING_PATCHES:use-nxp-bsp = "${TDX_UBOOT_HARDENING_PATCHES_DOWNSTREAM}"
TDX_UBOOT_HARDENING_PATCHES:verdin-am62 = "${TDX_UBOOT_HARDENING_PATCHES_DOWNSTREAM_AM62}"
TDX_UBOOT_HARDENING_PATCHES:verdin-am62-k3r5 = "${TDX_UBOOT_HARDENING_PATCHES_DOWNSTREAM_AM62}"

SRC_URI:append = "\
    file://u-boot-harden.cfg \
    ${TDX_UBOOT_HARDENING_PATCHES} \
"

do_compile:prepend:imx-generic-bsp() {
    if [ "${TDX_IMX_HAB_ENABLE}" = "0" ] && [ "${TDX_UBOOT_HARDENING_ENABLE}" = "1" ]; then
        bbfatal 'The combination TDX_IMX_HAB_ENABLE="0" and TDX_UBOOT_HARDENING_ENABLE="1" is not allowed:' \
                'the whitelisting feature (part of the hardening) currently relies on HAB/AHAB.'
    fi
}

do_compile:prepend:k3() {
    if [ "${TDX_K3_HSSE_ENABLE}" = "0" ] && [ "${TDX_UBOOT_HARDENING_ENABLE}" = "1" ]; then
        bbfatal 'The combination TDX_K3_HSSE_ENABLE="0" and TDX_UBOOT_HARDENING_ENABLE="1" is not allowed:' \
                'the whitelisting feature (part of the hardening) currently relies on K3.'
    fi
}

do_compile:prepend:k3r5() {
    # TODO: Consider implementing some kind of hardening for the R5 processor.
    if [ "${TDX_UBOOT_HARDENING_ENABLE}" = "1" ]; then
        bbfatal 'The hardening cannot be enabled for the R5 processor at the moment;' \
	        'please review your setting of TDX_UBOOT_HARDENING_ENABLE.'
    fi
}
