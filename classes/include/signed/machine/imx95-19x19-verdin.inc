# entrypoint and load addresses for FIT image generation
UBOOT_ENTRYPOINT = "0x94400000"
UBOOT_DTB_LOADADDRESS = "0x9c400000"
UBOOT_RD_LOADADDRESS = "0x9c800000"

# a custom wic description file is required when building
# dm-verity based rootfs image
WKS_FILE:tdx-signed-dmverity = "imx95-19x19-verdin.wks.in"
