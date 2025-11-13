FILESEXTRAPATHS:prepend := "${THISDIR}/files:"

SRC_URI += "\
    file://tee-supplicant.rules \
"

# The optee-client upstream recipe installs a tee-supplicant template unit
# that requires an udev rule to start the tee supplicant daemon automatically
do_install:append() {
    if [ -e "${D}${systemd_system_unitdir}/tee-supplicant@.service" ]; then
        install -d ${D}${nonarch_base_libdir}/udev/rules.d/
        install -m 755 ${UNPACKDIR}/tee-supplicant.rules ${D}${nonarch_base_libdir}/udev/rules.d/
    fi
}

# additional build flags for RPMB support in NXP downstream OP-TEE client (Makefile based)
EXTRA_OEMAKE:append = "\
    ${@oe.utils.conditional('TDX_OPTEE_DEBUG', '1', 'CFG_TEE_SUPP_LOG_LEVEL=3', '', d)} \
    CFG_TEE_FS_PARENT_PATH='${TDX_OPTEE_FS_PARENT_PATH}' \
"

# additional build flags for RPMB support in upstream OP-TEE client (CMake based)
EXTRA_OECMAKE:append = "\
    ${@oe.utils.conditional('TDX_OPTEE_DEBUG', '1', '-DCFG_TEE_SUPP_LOG_LEVEL=3', '', d)} \
    -DCFG_TEE_FS_PARENT_PATH='${TDX_OPTEE_FS_PARENT_PATH}' \
"

require ${@oe.utils.conditional('TDX_OPTEE_FS_RPMB', '1', 'optee-fs-rpmb.inc', '', d)}

# make sure the recipe is selected by the current machine
COMPATIBLE_MACHINE = "${MACHINE}"
