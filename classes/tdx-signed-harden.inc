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
# Moreover, currently the hardening is tested only with Torizon OS and it's
# not expected to work out of the box with BSP images.
#
# Because of the above, we enable the hardening only when building Torizon OS
# images provided that both TDX_IMX_HAB_ENABLE and UBOOT_SIGN_ENABLE are set, by
# default.
#
def default_tdx_uboot_hardening_enable(d):
    # TODO: Remove this condition once support for BSP images is added.
    if "torizon" not in d.getVar("OVERRIDES").split(":"):
        return False
    if (bb.utils.to_boolean(d.getVar('TDX_IMX_HAB_ENABLE')) and
        bb.utils.to_boolean(d.getVar('UBOOT_SIGN_ENABLE'))):
        return True
    return False

TDX_UBOOT_HARDENING_ENABLE ?= "${@'1' if default_tdx_uboot_hardening_enable(d) else '0'}"

# Configure one of the hardening features: the "bootargs protection"; with this
# protection, the fixed part of the kernel command line (boot arguments) are
# saved into the FIT image and checked against the "bootargs" environment
# variable at runtime (by U-Boot).
#
# Notice that the values set below are appropriate for booting Torizon OS only;
# this will be changed in the future.
#
# TODO: Set TDX_SECBOOT_REQUIRED_BOOTARGS differently based on overrides
# "tdx-signed" or "torizon-signed"; the former for booting the BSP reference
# image and the latter for Torizon OS.
#
# NOTE: TDX_SECBOOT_REQUIRED_BOOTARGS_COMMON should be kept in sync with
#       OSTREE_KERNEL_ARGS from layer meta-toradex-torizon (this is not done
#       here because meta-toradex-security does not depend on that layer).
#
TDX_SECBOOT_REQUIRED_BOOTARGS ?= ""
TDX_SECBOOT_REQUIRED_BOOTARGS_COMMON = "quiet logo.nologo vt.global_cursor_default=0 plymouth.ignore-serial-consoles splash fbcon=map:3"
TDX_SECBOOT_REQUIRED_BOOTARGS_COMMON:append:cfs-support = " systemd.gpt_auto=0"

TDX_SECBOOT_REQUIRED_BOOTARGS:imx-generic-bsp = "root=LABEL=otaroot rootfstype=ext4 ${TDX_SECBOOT_REQUIRED_BOOTARGS_COMMON}"
TDX_SECBOOT_REQUIRED_BOOTARGS:apalis-imx6 = "enable_wait_mode=off vmalloc=400M root=LABEL=otaroot rootfstype=ext4 ${TDX_SECBOOT_REQUIRED_BOOTARGS_COMMON}"
TDX_SECBOOT_REQUIRED_BOOTARGS:colibri-imx6 = "enable_wait_mode=off galcore.contiguousSize=50331648 root=LABEL=otaroot rootfstype=ext4 ${TDX_SECBOOT_REQUIRED_BOOTARGS_COMMON}"
TDX_SECBOOT_REQUIRED_BOOTARGS:colibri-imx6ull-emmc = "user_debug=30 root=LABEL=otaroot rootfstype=ext4 ${TDX_SECBOOT_REQUIRED_BOOTARGS_COMMON}"

TDX_SECBOOT_REQUIRED_BOOTARGS:verdin-am62 = ""
TDX_SECBOOT_REQUIRED_BOOTARGS:qemuarm64 = ""

# Name of the overlay file (without extension) that will contain the fixed part
# of the kernel command line; this will be stored inside the FIT image; notice
# we add the machine prefix to ensure the overlay is identified as a machine-
# specific one and picked up by toradex-devicetree.bbclass.
TDX_SECBOOT_KARGS_OVERLAY = "${MACHINE}-secboot-kargs_overlay"
