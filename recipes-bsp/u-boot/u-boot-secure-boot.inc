FILESEXTRAPATHS:prepend := "${THISDIR}/files:"

TDX_UBOOT_SECBOOT_PATCHES_COMMON = " \
    file://0001-toradex-common-add-shared-secboot-module.patch \
"

TDX_UBOOT_SECBOOT_PATCHES_UPSTREAM = " \
    ${TDX_UBOOT_SECBOOT_PATCHES_COMMON} \
    file://0001-lmb-prevent-alloc-overlap-with-reserved-regions-upst.patch \
    file://0001-toradex-integrate-shared-secboot-module-upstream.patch \
"

# NOTE: NXP downstream does not need patch "lmb: prevent alloc overlap with reserved regions"
TDX_UBOOT_SECBOOT_PATCHES_DOWNSTREAM = " \
    ${TDX_UBOOT_SECBOOT_PATCHES_COMMON} \
    file://0001-toradex-add-helper-to-detect-closed-iMX95-downstream.patch \
    file://0001-toradex-integrate-shared-secboot-module-downstream.patch \
"

TDX_UBOOT_SECBOOT_PATCHES_DOWNSTREAM_K3 = " \
    ${TDX_UBOOT_SECBOOT_PATCHES_COMMON} \
    file://0001-lmb-prevent-alloc-overlap-with-reserved-regions-k3.patch \
    file://0001-toradex-add-helper-to-detect-closed-device-k3.patch \
    file://0001-toradex-integrate-shared-secboot-module-downstream.patch \
"

TDX_UBOOT_SECBOOT_PATCHES = "${TDX_UBOOT_SECBOOT_PATCHES_UPSTREAM}"
TDX_UBOOT_SECBOOT_PATCHES:apalis-imx8 = "${TDX_UBOOT_SECBOOT_PATCHES_DOWNSTREAM}"
TDX_UBOOT_SECBOOT_PATCHES:colibri-imx8x = "${TDX_UBOOT_SECBOOT_PATCHES_DOWNSTREAM}"
TDX_UBOOT_SECBOOT_PATCHES:toradex-smarc-imx95 = "${TDX_UBOOT_SECBOOT_PATCHES_DOWNSTREAM}"
TDX_UBOOT_SECBOOT_PATCHES:imx95-19x19-verdin = "${TDX_UBOOT_SECBOOT_PATCHES_DOWNSTREAM}"
TDX_UBOOT_SECBOOT_PATCHES:k3 = "${TDX_UBOOT_SECBOOT_PATCHES_DOWNSTREAM_K3}"
TDX_UBOOT_SECBOOT_PATCHES:k3r5 = ""

SRC_URI:append = " \
    ${TDX_UBOOT_SECBOOT_PATCHES} \
"

# enable FIT image signature
require ${@oe.utils.conditional('UBOOT_SIGN_ENABLE', '1', 'u-boot-fit-signature.inc', '', d)}

# enable HAB support
require ${@oe.utils.conditional('TDX_IMX_HAB_ENABLE', '1', 'u-boot-hab.inc', '', d)}

# enable TI K3 secure boot support
require ${@oe.utils.conditional('TDX_K3_SECBOOT_ENABLE', '1', 'u-boot-k3-secboot.inc', '', d)}

# enable U-Boot hardening
require ${@oe.utils.conditional('TDX_UBOOT_HARDENING_ENABLE', '1', 'u-boot-harden.inc', '', d)}
