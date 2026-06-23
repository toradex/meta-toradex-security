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
    unsupported_machines = ["imx95-19x19-verdin"]
    for machine in unsupported_machines:
        if (machine in d.getVar('OVERRIDES').split(':')):
            bb.warnonce("%s machine requires some additional work to have "\
                        "out-of-the-box support for U-Boot hardening, so " \
                        "it is currently disabled by default." % machine)
            return False
    if ('imx-generic-bsp' in d.getVar('OVERRIDES').split(':') and
        bb.utils.to_boolean(d.getVar('TDX_IMX_HAB_ENABLE')) and
        bb.utils.to_boolean(d.getVar('UBOOT_SIGN_ENABLE'))):
        return True
    if ('k3' in d.getVar('OVERRIDES').split(':') and
        bb.utils.to_boolean(d.getVar('TDX_K3_SECBOOT_ENABLE')) and
        bb.utils.to_boolean(d.getVar('UBOOT_SIGN_ENABLE'))):
        return True
    return False

TDX_UBOOT_HARDENING_ENABLE ?= "${@'1' if default_tdx_uboot_hardening_enable(d) else '0'}"
TDX_UBOOT_HARDENING_ENABLE:k3r5 = "0"
TDX_UBOOT_HARDENING_ENABLE_DBG ?= "0"
TDX_UBOOT_HARDENING_ENABLE_CP_PROT ?= "0"
TDX_UBOOT_HARDENING_ENABLE_FB_PROT ?= "0"

# Configure the "kernel command-line protection"; with this protection, the
# fixed part of the bootargs are saved into the FIT image and checked against
# the "bootargs" environment variable at runtime (by U-Boot).
#
# Per-machine default bootargs for BSP images:
TDX_SECBOOT_REQUIRED_BOOTARGS_DEFAULT ?= ""
TDX_SECBOOT_REQUIRED_BOOTARGS_DEFAULT:apalis-imx6 ?= "ro rootwait console=tty1 console=ttymxc0,115200"
TDX_SECBOOT_REQUIRED_BOOTARGS_DEFAULT:colibri-imx6 ?= "ro rootwait console=tty1 console=ttymxc0,115200"
TDX_SECBOOT_REQUIRED_BOOTARGS_DEFAULT:colibri-imx6ull-emmc ?= "ro rootwait console=tty1 console=ttymxc0,115200"
TDX_SECBOOT_REQUIRED_BOOTARGS_DEFAULT:colibri-imx7-emmc ?= "ro rootwait console=tty1 console=ttymxc0,115200"
TDX_SECBOOT_REQUIRED_BOOTARGS_DEFAULT:apalis-imx8 ?= "ro rootwait console=tty1 console=ttyLP1,115200"
TDX_SECBOOT_REQUIRED_BOOTARGS_DEFAULT:verdin-imx8mm ?= "ro rootwait console=tty1 console=ttymxc0,115200"
TDX_SECBOOT_REQUIRED_BOOTARGS_DEFAULT:verdin-imx8mp ?= "ro rootwait console=tty1 console=ttymxc2,115200"
TDX_SECBOOT_REQUIRED_BOOTARGS_DEFAULT:colibri-imx8x ?= "ro rootwait console=tty1 console=ttyLP3,115200"
TDX_SECBOOT_REQUIRED_BOOTARGS_DEFAULT:verdin-am62 ?= "ro rootwait console=tty1 console=ttyS2,115200"
TDX_SECBOOT_REQUIRED_BOOTARGS_DEFAULT:verdin-am62p ?= "ro rootwait console=tty1 console=ttyS2,115200"
TDX_SECBOOT_REQUIRED_BOOTARGS_DEFAULT:aquila-am69 ?= "ro rootwait console=tty1 console=ttyS2,115200"
TDX_SECBOOT_REQUIRED_BOOTARGS_DEFAULT:toradex-smarc-imx95 ?= "ro rootwait console=tty1 console=ttyLP1,115200"
TDX_SECBOOT_REQUIRED_BOOTARGS_DEFAULT:toradex-smarc-imx8mp ?= "ro rootwait console=tty1 console=ttymxc1,115200"
TDX_SECBOOT_REQUIRED_BOOTARGS_DEFAULT:verdin-imx95 ?= "ro rootwait console=tty1 console=ttyLP2,115200"
TDX_SECBOOT_REQUIRED_BOOTARGS_DEFAULT:aquila-imx95 ?= "ro rootwait console=tty1 console=ttyLP2,115200"
# Bootargs for BSP images; users should override this one when needed:
TDX_SECBOOT_REQUIRED_BOOTARGS ?= "${TDX_SECBOOT_REQUIRED_BOOTARGS_DEFAULT}"

# Name of the overlay file (without extension) that will contain the fixed part
# of the kernel command line; this will be stored inside the FIT image; notice
# we add the machine prefix to ensure the overlay is identified as a machine-
# specific one and picked up by toradex-devicetree.bbclass.
TDX_SECBOOT_KARGS_OVERLAY = "${MACHINE}-secboot-kargs_overlay"
