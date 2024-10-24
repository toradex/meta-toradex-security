# Enable the Secure Boot hardening of U-Boot by Toradex.
#
# Currently the hardening implementation requires HAB/AHAB (or equivalent
# technology in TI SoCs) to be enabled so that the combination
# TDX_IMX_HAB_ENABLE="0" and TDX_UBOOT_HARDENING_ENABLE="1" is not valid.
#
# Also the hardening itself is supposed to protect the second link of the chain
# of trust i.e. the link between bootloader and the kernel artifacts. So, the
# combination UBOOT_SIGN_ENABLE="0" TDX_UBOOT_HARDENING_ENABLE="1" is not very
# helpful and can prevent the loading of non-FIT images.
#
# Because of the above, we enable the hardening only when TDX_IMX_HAB_ENABLE and
# UBOOT_SIGN_ENABLE are set, by default.
#
def default_tdx_uboot_hardening_enable(d):
    if ('imx-generic-bsp' in d.getVar('OVERRIDES').split(':') and
        bb.utils.to_boolean(d.getVar('TDX_IMX_HAB_ENABLE')) and
        bb.utils.to_boolean(d.getVar('UBOOT_SIGN_ENABLE'))):
        return True
    if ('k3' in d.getVar('OVERRIDES').split(':') and
        bb.utils.to_boolean(d.getVar('TDX_K3_HSSE_ENABLE')) and
        bb.utils.to_boolean(d.getVar('UBOOT_SIGN_ENABLE'))):
        return True
    return False

TDX_UBOOT_HARDENING_ENABLE ?= "${@'1' if default_tdx_uboot_hardening_enable(d) else '0'}"
TDX_UBOOT_HARDENING_ENABLE:verdin-am62-k3r5 = "0"

# Configure the "kernel command-line protection"; with this protection, the
# fixed part of the bootargs are saved into the FIT image and checked against
# the "bootargs" environment variable at runtime (by U-Boot).
#
# TODO: Ensure bootargs are set correctly on all machines.
TDX_SECBOOT_REQUIRED_BOOTARGS ?= ""
TDX_SECBOOT_REQUIRED_BOOTARGS:apalis-imx6 ?= "ro rootwait console=tty1 console=ttymxc0,115200"
TDX_SECBOOT_REQUIRED_BOOTARGS:colibri-imx6 ?= "ro rootwait console=tty1 console=ttymxc0,115200"
TDX_SECBOOT_REQUIRED_BOOTARGS:colibri-imx6ull-emmc ?= "ro rootwait console=tty1 console=ttymxc0,115200"
TDX_SECBOOT_REQUIRED_BOOTARGS:colibri-imx7 ?= "ro rootwait console=tty1 console=ttymxc0,115200"
TDX_SECBOOT_REQUIRED_BOOTARGS:apalis-imx8 ?= "ro rootwait console=tty1 console=ttyLP1,115200"
TDX_SECBOOT_REQUIRED_BOOTARGS:verdin-imx8mm ?= "ro rootwait console=tty1 console=ttymxc0,115200"
TDX_SECBOOT_REQUIRED_BOOTARGS:verdin-imx8mp ?= "ro rootwait console=tty1 console=ttymxc2,115200"
TDX_SECBOOT_REQUIRED_BOOTARGS:colibri-imx8x ?= "ro rootwait console=tty1 console=ttyLP3,115200"
TDX_SECBOOT_REQUIRED_BOOTARGS:verdin-am62 ?= "ro rootwait console=tty1 console=ttyS2,115200"

# Name of the overlay file (without extension) that will contain the fixed part
# of the kernel command line; this will be stored inside the FIT image; notice
# we add the machine prefix to ensure the overlay is identified as a machine-
# specific one and picked up by toradex-devicetree.bbclass.
TDX_SECBOOT_KARGS_OVERLAY = "${MACHINE}-secboot-kargs_overlay"
