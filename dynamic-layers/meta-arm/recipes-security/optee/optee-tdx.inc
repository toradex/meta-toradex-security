COMPATIBLE_MACHINE = "${MACHINE}"

# include machine specific configuration
require include/optee-tdx-${MACHINE}.inc

# make options to enable debug messages
OPTEE_OS_DEBUG_OPTS = "\
    CFG_TEE_CORE_LOG_LEVEL=3 \
    CFG_TEE_TA_LOG_LEVEL=3 \
    CFG_TEE_CORE_TA_TRACE=y \
    CFG_TEE_CORE_DEBUG=y \
"

# additional make options to support encryption
OPTEE_OS_ENCRYPTION_OPTS = ""
OPTEE_OS_ENCRYPTION_OPTS:tdx-encrypted = "\
    CFG_IN_TREE_EARLY_TAS=trusted_keys/f04a0fe7-1f5d-4b9b-abf7-619b85b4ce8c \
    CFG_WITH_SOFTWARE_PRNG=n \
    CFG_HWRNG_PTA=y \
    CFG_HWRNG_QUALITY=1024 \
"

# additional build flags for optee-os
EXTRA_OEMAKE:append:pn-optee-os = "\
    ${@oe.utils.conditional('TDX_OPTEE_PKCS11', '1', 'CFG_PKCS11_TA=y', '', d)} \
    ${@oe.utils.conditional('TDX_OPTEE_DEBUG', '1', '${OPTEE_OS_DEBUG_OPTS}', '', d)} \
    ${@oe.utils.conditional('TDX_ENC_KEY_BACKEND', 'tee', '${OPTEE_OS_ENCRYPTION_OPTS}', '', d)} \
    CFG_UART_BASE=${UART_BASE_ADDR} \
    ${OPTEE_OS_EXTRA_MACHINE_ARGS} \
"

require ${@oe.utils.conditional('TDX_OPTEE_FS_RPMB', '1', 'optee-fs-rpmb.inc', '', d)}
