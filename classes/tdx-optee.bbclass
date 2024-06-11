# enable OP-TEE support
TDX_OPTEE_ENABLE = "1"

# required by some vendor BSPs
MACHINE_FEATURES:append = " optee"

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
    ${@oe.utils.conditional('TDX_OPTEE_INSTALL_TESTS', '1', '${TDX_OPTEE_PACKAGES_TESTS}', '', d)} \
"

# validate optee support
addhandler validate_optee_support
validate_optee_support[eventmask] = "bb.event.SanityCheck"
python validate_optee_support() {
    supported_machines = ['verdin-imx8mp']
    machine = e.data.getVar('MACHINE')
    if machine not in supported_machines:
        bb.fatal("OP-TEE is currently not supported on '%s' machine!" % machine)

    if 'tdx-signed-dmverity' in d.getVar('OVERRIDES').split(':'):
        bb.fatal("Currently, OP-TEE cannot be used together with dm-verity because it needs a writable rootfs!")
}
