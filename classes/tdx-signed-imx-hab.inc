# Enable iMX HAB/AHAB.
TDX_IMX_HAB_ENABLE ?= "1"

# Enable the Secure Boot hardening of U-Boot by Toradex.
#
# Currently the hardening implementation requires HAB/AHAB to be enabled so that
# the combination TDX_IMX_HAB_ENABLE="0" TDX_UBOOT_HARDENING_ENABLE="1" is not
# valid.
#
# Also the hardening itself is supposed to protect the second link of the chain
# of trust i.e. the link between bootloader and the kernel artifacts. So, the
# combination UBOOT_SIGN_ENABLE="0" TDX_UBOOT_HARDENING_ENABLE="1" is not very
# helpful and can prevent the loading of non-FIT images.
#
# Because of the above, we enable the hardening only if both TDX_IMX_HAB_ENABLE
# and UBOOT_SIGN_ENABLE are both set, by default.
#
TDX_UBOOT_HARDENING_ENABLE ?= "${@'1' if bb.utils.to_boolean(d.getVar('TDX_IMX_HAB_ENABLE')) and bb.utils.to_boolean(d.getVar('UBOOT_SIGN_ENABLE')) else '0'}"
