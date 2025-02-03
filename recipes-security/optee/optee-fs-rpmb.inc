# additional build flags for RPMB support in OP-TEE OS
EXTRA_OEMAKE:append:pn-optee-os = "\
    CFG_RPMB_FS=y \
    CFG_REE_FS=n \
    CFG_RPMB_FS_DEV_ID=${TDX_OPTEE_FS_RPMB_DEV_ID} \
    ${@oe.utils.conditional('TDX_OPTEE_FS_RPMB_MODE', 'development', 'CFG_RPMB_WRITE_KEY=y', '', d)} \
    ${@oe.utils.conditional('TDX_OPTEE_FS_RPMB_MODE', 'factory', 'CFG_RPMB_WRITE_KEY=y CFG_RPMB_RESET_FAT=y', '', d)} \
    ${@oe.utils.conditional('TDX_OPTEE_FS_RPMB_MODE', 'production', '', '', d)} \
"

# additional build flags for RPMB support in upstream OP-TEE client (CMake based)
OPTEE_CLIENT_RPMB_CMAKE_OPTS = "\
    ${@oe.utils.conditional('TDX_OPTEE_FS_RPMB_MODE', 'development', '-DRPMB_EMU=ON', '', d)} \
    ${@oe.utils.conditional('TDX_OPTEE_FS_RPMB_MODE', 'factory', '-DRPMB_EMU=OFF', '', d)} \
    ${@oe.utils.conditional('TDX_OPTEE_FS_RPMB_MODE', 'production', '-DRPMB_EMU=OFF', '', d)} \
"
OPTEE_CLIENT_RPMB_CMAKE_OPTS:imx-generic-bsp = ""
EXTRA_OECMAKE:append:pn-optee-client = "\
    ${OPTEE_CLIENT_RPMB_CMAKE_OPTS} \
"

# additional build flags for RPMB support in NXP downstream OP-TEE client (Makefile based)
OPTEE_CLIENT_RPMB_MAKE_OPTS = ""
OPTEE_CLIENT_RPMB_MAKE_OPTS:imx-generic-bsp = "\
    ${@oe.utils.conditional('TDX_OPTEE_FS_RPMB_MODE', 'development', 'RPMB_EMU=1', '', d)} \
    ${@oe.utils.conditional('TDX_OPTEE_FS_RPMB_MODE', 'factory', 'RPMB_EMU=0', '', d)} \
    ${@oe.utils.conditional('TDX_OPTEE_FS_RPMB_MODE', 'production', 'RPMB_EMU=0', '', d)} \
"
EXTRA_OEMAKE:append:pn-optee-client = "\
    ${OPTEE_CLIENT_RPMB_MAKE_OPTS} \
"

# The plat_rpmb_key_is_ready() function in OP-TEE is a platform-specific hook that
# indicates whether the RPMB key is ready to be programmed and used. On i.MX platforms,
# this function is implemented to prevent the use of RPMB if the device is not in a
# closed state (i.e., secure boot enabled). In development mode, where the RPMB partition
# is emulated in software, this behavior is unnecessary. Therefore, the function can be
# disabled to allow RPMB emulation to operate without restrictions.
do_compile:prepend:pn-optee-os() {
    if [ ${TDX_OPTEE_FS_RPMB_MODE} = "development" ]; then
        sed -i 's/plat_rpmb_key_is_ready/_plat_rpmb_key_disabled/g' ${S}/core/drivers/imx_snvs.c
    fi
}
