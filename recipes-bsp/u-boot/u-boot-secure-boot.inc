FILESEXTRAPATHS:prepend := "${THISDIR}/files:"

# enable FIT image signature
require ${@oe.utils.conditional('UBOOT_SIGN_ENABLE', '1', 'u-boot-fit-signature.inc', '', d)}
