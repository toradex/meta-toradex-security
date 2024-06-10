COMPATIBLE_MACHINE = "${MACHINE}"

# include machine specific configuration
require optee-tdx-${MACHINE}.inc

# make options to enable debug messages
OPTEE_OS_DEBUG_OPTS = "\
    CFG_TEE_CORE_LOG_LEVEL=3 \
    CFG_TEE_TA_LOG_LEVEL=3 \
"

# additional build flags for optee-os
EXTRA_OEMAKE:append:pn-optee-os = "\
    ${@oe.utils.conditional('TDX_OPTEE_DEBUG', '1', '${OPTEE_OS_DEBUG_OPTS}', '', d)} \
    CFG_UART_BASE=${UART_BASE_ADDR} \
"
