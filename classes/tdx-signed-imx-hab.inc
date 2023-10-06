# enable iMX HAB/AHAB
TDX_IMX_HAB_ENABLE ?= "1"

# enable the Secure Boot hardening of U-Boot by Toradex; currently the U-Boot
# hardening requires HAB to be enabled so that the combination
# TDX_IMX_HAB_ENABLE="0" TDX_UBOOT_HARDENING_ENABLE="1" is not valid
TDX_UBOOT_HARDENING_ENABLE ?= "${TDX_IMX_HAB_ENABLE}"
