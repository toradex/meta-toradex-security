# compatible machine in OP-TEE source code
OPTEEMACHINE = "imx-mx8qmmek"

# required by NXP OP-TEE recipe
PLATFORM_FLAVOR = "mx8qmmek"

# uart base address (to print debug messages)
UART_BASE_ADDR = "0x5a070000"

# extra arguments to build OP-TEE OS
OPTEE_OS_EXTRA_MACHINE_ARGS = "\
    CFG_TZDRAM_START=${TDX_OPTEE_ADDR_LOAD} \
    CFG_TZDRAM_SIZE=${TDX_OPTEE_ADDR_SIZE} \
    CFG_NS_ENTRY_ADDR=0x95400000 \
    CFG_DT=n \
"
