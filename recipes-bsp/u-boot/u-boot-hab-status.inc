FILESEXTRAPATHS:prepend := "${THISDIR}/files/u-boot-hab-status:"

TDX_UBOOT_HAB_STATUS_PATCHES_COMMON = " \
    file://0001-toradex-common-add-hab-utilities-module.patch \
"

TDX_UBOOT_HAB_STATUS_PATCHES_HAB = " \
    ${TDX_UBOOT_HAB_STATUS_PATCHES_COMMON} \
    file://0001-toradex-adjust-hab_status-exit-code-behavior.patch \
"

TDX_UBOOT_HAB_STATUS_PATCHES_SECO = " \
    ${TDX_UBOOT_HAB_STATUS_PATCHES_COMMON} \
    file://0001-toradex-adjust-ahab_status-exit-code-behavior-seco.patch \
"

TDX_UBOOT_HAB_STATUS_PATCHES_ELE = " \
    ${TDX_UBOOT_HAB_STATUS_PATCHES_COMMON} \
    file://0001-toradex-adjust-ahab_status-exit-code-behavior-ele.patch \
"

TDX_UBOOT_HAB_STATUS_PATCHES = "${TDX_UBOOT_HAB_STATUS_PATCHES_HAB}"
TDX_UBOOT_HAB_STATUS_PATCHES:apalis-imx8 = "${TDX_UBOOT_HAB_STATUS_PATCHES_SECO}"
TDX_UBOOT_HAB_STATUS_PATCHES:colibri-imx8x = "${TDX_UBOOT_HAB_STATUS_PATCHES_SECO}"
TDX_UBOOT_HAB_STATUS_PATCHES:mx9-generic-bsp = "${TDX_UBOOT_HAB_STATUS_PATCHES_ELE}"

SRC_URI:append = " \
    ${TDX_UBOOT_HAB_STATUS_PATCHES} \
"
