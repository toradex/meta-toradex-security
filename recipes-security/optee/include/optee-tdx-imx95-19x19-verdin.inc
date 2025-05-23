# compatible machine in OP-TEE source code
OPTEEMACHINE = "imx-mx95evk"

# uart base address (to print debug messages)
UART_BASE_ADDR = "0x44380000"

# eMMC RPMB partition device node ID
TDX_OPTEE_FS_RPMB_DEV_ID ?= "0"

# additional build parameters for encryption support
OPTEE_OS_EXTRA_MACHINE_ARGS:append:tdx-encrypted = "\
    ${@oe.utils.conditional('TDX_ENC_KEY_BACKEND', 'tee', 'CFG_IMX_TRUSTED_ARM_CE=y', '', d)} \
"
