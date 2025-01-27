FILESEXTRAPATHS:prepend := "${THISDIR}/files:"

SRC_URI += "\
    file://tee-supplicant.rules \
"

# The optee-client upstream recipe installs a tee-supplicant template unit
# that requires an udev rule to start the tee supplicant daemon automatically
do_install:append() {
    if [ -e "${D}${systemd_system_unitdir}/tee-supplicant@.service" ]; then
        install -d ${D}${nonarch_base_libdir}/udev/rules.d/
        install -m 755 ${WORKDIR}/tee-supplicant.rules ${D}${nonarch_base_libdir}/udev/rules.d/
    fi
}

EXTRA_OEMAKE:append = "\
    ${@oe.utils.conditional('TDX_OPTEE_DEBUG', '1', 'CFG_TEE_SUPP_LOG_LEVEL=3', '', d)} \
"

require ${@oe.utils.conditional('TDX_OPTEE_FS_RPMB', '1', 'optee-fs-rpmb.inc', '', d)}

# make sure the recipe is selected by the current machine
COMPATIBLE_MACHINE = "${MACHINE}"
