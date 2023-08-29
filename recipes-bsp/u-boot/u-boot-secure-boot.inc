FILESEXTRAPATHS:prepend := "${THISDIR}/files:"

# enable FIT image signature
require ${@oe.utils.conditional('UBOOT_SIGN_ENABLE', '1', 'u-boot-fit-signature.inc', '', d)}

# enable HAB support
require ${@oe.utils.conditional('TDX_IMX_HAB_ENABLE', '1', 'u-boot-hab.inc', '', d)}

# enable U-Boot hardening
require ${@oe.utils.conditional('TDX_UBOOT_HARDENING_ENABLE', '1', 'u-boot-harden.inc', '', d)}
