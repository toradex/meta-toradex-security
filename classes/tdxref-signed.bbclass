# sign bootloader and kernel artifacts
inherit tdx-signed

# boot a signed rootfs via dm-verity
inherit tdx-signed-dmverity
