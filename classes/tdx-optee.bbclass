# enable OP-TEE support
TDX_OPTEE_ENABLE = "1"

# disable OP-TEE on R5 firmware for K3 based platforms
TDX_OPTEE_ENABLE:verdin-am62-k3r5 = "0"

# required by some vendor BSPs
MACHINE_FEATURES:append = " optee"

# enable support for building OP-TEE with the fTPM trusted application
TDX_OPTEE_FTPM ?= "0"
MACHINE_FEATURES:append = " ${@oe.utils.conditional('TDX_OPTEE_FTPM', '1', 'optee-ftpm', '', d)}"

# enable OP-TEE debug messages
TDX_OPTEE_DEBUG ?= "0"

# enable installation of OP-TEE test applications
TDX_OPTEE_INSTALL_TESTS ?= "0"

# OP-TEE test applications
TDX_OPTEE_PACKAGES_TESTS = "\
    optee-test \
    optee-examples \
"

# extra packages for OP-TEE support
IMAGE_INSTALL:append = "\
    optee-os \
    optee-client \
    ${@oe.utils.conditional('TDX_OPTEE_FTPM', '1', 'tpm2-tools', '', d)} \
    ${@oe.utils.conditional('TDX_OPTEE_INSTALL_TESTS', '1', '${TDX_OPTEE_PACKAGES_TESTS}', '', d)} \
"

# enable data partition if dm-verity and tezi are both enabled
inherit ${@ 'tdx-tezi-data-partition' if 'teziimg' in d.getVar('IMAGE_FSTYPES') and \
            'tdx-signed-dmverity' in d.getVar('OVERRIDES').split(':') else ''}

# validate optee support
addhandler validate_optee_support
validate_optee_support[eventmask] = "bb.event.SanityCheck"
python validate_optee_support() {
    supported_machines = [
        'verdin-imx8mp',
        'verdin-imx8mm',
        'verdin-am62',
    ]

    if e.data.getVar('TDX_OPTEE_ENABLE') == '0':
        return

    machine = e.data.getVar('MACHINE')
    if machine not in supported_machines:
        bb.fatal("OP-TEE is currently not supported on '%s' machine!" % machine)
}
