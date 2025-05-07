# compatible machine in OP-TEE source code
OPTEEMACHINE = "imx-mx95evk"

# uart base address (to print debug messages)
UART_BASE_ADDR = "0x44380000"

# eMMC RPMB partition device node ID
TDX_OPTEE_FS_RPMB_DEV_ID ?= "0"
