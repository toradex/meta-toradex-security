# sign bootloader and kernel artifacts
inherit tdx-signed

# boot a signed rootfs via dm-verity
inherit tdx-signed-dmverity

# override signing class name (informational, may be displayed to users)
TDX_SIGNING_CLASS = "tdxref-signed"
