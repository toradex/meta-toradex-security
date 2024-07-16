# When hardening is enabled, ask for the use of the overlay containing the
# "secure" bootargs. Here we assume an underlying layer will provide this
# capabitlity of applying predefined overlays inside the FIT image based on
# variable DISTRO_BOOT_PREDEF_FITCONF; such capability would normally rely
# on the boot script.
#
# For Torizon OS, the handling of predefined overlays is done by the
# u-boot-distro-boot recipe provided by layer meta-toradex-torizon.
#
# TODO: Handle this variable also in the BSP layer.
#
DISTRO_BOOT_PREDEF_FITCONF .= "${@oe.utils.conditional('TDX_UBOOT_HARDENING_ENABLE', '1', '#conf-${TDX_SECBOOT_KARGS_OVERLAY}.dtbo', '', d)}"
