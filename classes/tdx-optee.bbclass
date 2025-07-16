# enable OP-TEE support
TDX_OPTEE_ENABLE = "1"

# disable OP-TEE on R5 firmware for K3 based platforms
TDX_OPTEE_ENABLE:k3r5 = "0"

# required by some vendor BSPs
MACHINE_FEATURES:append = " optee"

# enable support for building OP-TEE with the fTPM trusted application
TDX_OPTEE_FTPM ?= "0"
MACHINE_FEATURES:append = " ${@oe.utils.conditional('TDX_OPTEE_FTPM', '1', 'optee-ftpm', '', d)}"

# enable support for PKCS#11 trusted application
TDX_OPTEE_PKCS11 ?= "0"

# root directory when using the REE filesystem for storage
TDX_OPTEE_FS_PARENT_PATH ?= "/data/tee"

# enable support for using the eMMC RPMB partition as a secure storage device
TDX_OPTEE_FS_RPMB ?= "0"

# eMMC RPMB partition device node ID (i.e. /dev/mmcblk<id>rpmb)
TDX_OPTEE_FS_RPMB_DEV_ID ??= "0"

# RPMB secure storage operation mode
#    "development": The RPMB partition is emulated in software by tee-supplicant.
#                   Be aware that the emulation is done in memory, so the
#                   contents of the storage is lost after a reboot or when the
#                   tee-supplicant daemon is restarted.
#    "factory":     The eMMC RPMB partition is used, and OP-TEE is configured
#                   to program the RPMB key. The RPMB key is derived from the
#                   SoC's Hardware Unique Key (HUK). This mode is intended for
#                   secure factory provisioning environments, and it should
#                   never be used outside of a trusted, secure factory setup.
#    "production":  The eMMC RPMB partition is used, and it is assumed that the
#                   RPMB key has already been programmed into the RPMB partition.
#                   This mode is designed for production environments, where
#                   secure storage operations rely on a pre-provisioned RPMB key.
TDX_OPTEE_FS_RPMB_MODE ?= "development"

# enable OP-TEE debug messages
TDX_OPTEE_DEBUG ?= "0"

# enable installation of OP-TEE test applications
TDX_OPTEE_INSTALL_TESTS ?= "0"

# OP-TEE test applications
TDX_OPTEE_PACKAGES_TESTS = "\
    optee-test \
    optee-examples \
    mmc-utils \
    openssl-bin \
"

# extra packages for OP-TEE support
IMAGE_INSTALL:append = "\
    optee-os \
    optee-client \
    ${@oe.utils.conditional('TDX_OPTEE_FTPM', '1', 'tpm2-tools', '', d)} \
    ${@oe.utils.conditional('TDX_OPTEE_PKCS11', '1', 'libp11 opensc', '', d)} \
    ${@oe.utils.conditional('TDX_OPTEE_INSTALL_TESTS', '1', '${TDX_OPTEE_PACKAGES_TESTS}', '', d)} \
"

# enable data partition if dm-verity and tezi are both enabled, and RPMB is not enabled
inherit ${@ 'tdx-tezi-data-partition' if 'teziimg' in d.getVar('IMAGE_FSTYPES') and \
            'tdx-signed-dmverity' in d.getVar('OVERRIDES').split(':') and \
            d.getVar('TDX_OPTEE_FS_RPMB') == '0' else ''}

# machine-specific fixups for OP-TEE
include include/optee/machine/${MACHINE}.inc

# validate optee support
addhandler validate_optee_support
validate_optee_support[eventmask] = "bb.event.SanityCheck"
python validate_optee_support() {
    supported_machines = [
        'apalis-imx6',
        'aquila-am69',
        'colibri-imx6',
        'imx95-19x19-verdin',
        'verdin-am62',
        'verdin-imx8mm',
        'verdin-imx8mp',
    ]

    if e.data.getVar('TDX_OPTEE_ENABLE') == '0':
        return

    machine = e.data.getVar('MACHINE')
    if machine not in supported_machines:
        bb.fatal("OP-TEE is currently not supported on '%s' machine!" % machine)

    if e.data.getVar('TDX_OPTEE_FS_RPMB') == '1' and e.data.getVar('TDX_OPTEE_FS_RPMB_MODE') == 'factory':
        bb.warn("The factory mode for OP-TEE RPMB support is intended for " \
                "secure factory provisioning environments, and it should " \
                "never be used outside of a trusted, secure factory setup!")

    # Currently, OP-TEE cannot be used in conjunction with HAB on iMX6 SoMs due to a limitation
    # in the signing process. A different U-Boot image is generated when OP-TEE is enabled, and the
    # signing scripts need to be adapted to handle it.
    if 'mx6-generic-bsp' in d.getVar('OVERRIDES').split(':') and e.data.getVar('TDX_IMX_HAB_ENABLE') == '1':
        bb.fatal("Currenly, OP-TEE cannot be used together with HAB on iMX6!")
}
