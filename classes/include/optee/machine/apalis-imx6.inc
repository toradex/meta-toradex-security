# extension to append to optee binary
OPTEE_BIN_EXT = "bin"

# OP-TEE load address
TDX_OPTEE_ADDR_LOAD = "0x1c000000"

# OP-TEE reserved memory
TDX_OPTEE_ADDR_SIZE ?= "0x1000000"

# add OP-TEE nodes to kernel device tree
require classes/include/optee/linux-dt-overlay.inc
