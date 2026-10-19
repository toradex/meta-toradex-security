# add OP-TEE firmware and OP-TEE configuration nodes to FIT image
TDX_OPTEE_ADD_FIT_NODES = "1"

# when booting from FIT image, OP-TEE doesn't need to be installed into /boot
IMAGE_BOOT_FILES:remove = "tee.bin uTee-bin"
