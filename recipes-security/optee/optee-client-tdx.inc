# additional build flags for OP-TEE client
EXTRA_OECMAKE:append = "\
    ${@oe.utils.conditional('TDX_OPTEE_DEBUG', '1', '-DCFG_TEE_SUPP_LOG_LEVEL=3', '', d)} \
    -DCFG_TEE_FS_PARENT_PATH='${TDX_OPTEE_FS_PARENT_PATH}' \
"

# secure storage is either in the RPMB partition or in the REE filesystem
require ${@oe.utils.conditional('TDX_OPTEE_FS_RPMB', '1', 'optee-fs-rpmb.inc', 'optee-fs-ree.inc', d)}

# make sure the recipe is selected by the current machine
COMPATIBLE_MACHINE = "${MACHINE}"
