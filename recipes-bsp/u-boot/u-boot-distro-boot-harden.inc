# When hardening is enabled, ask for the use of the overlay containing the
# "secure" bootargs. Here we assume an underlying layer will provide this
# capabitlity of applying predefined overlays inside the FIT image based on
# variable FITCONF_FDT_OVERLAYS; such capability would normally rely on the
# boot script.
#
# For Torizon OS, the handling of predefined overlays is done by the
# u-boot-distro-boot recipe provided by layer meta-toradex-torizon, whereas
# for Toradex BSP Reference images, this is done by the u-boot-distro-boot
# recipe provided by layer meta-toradex-bsp-common.
#
FITCONF_FDT_OVERLAYS .= "${@oe.utils.conditional('TDX_UBOOT_HARDENING_ENABLE', '1', '#conf-${TDX_SECBOOT_KARGS_OVERLAY}.dtbo', '', d)}"
