# a ramdisk is required to check the signature of the rootfs image
INITRAMFS_IMAGE ?= "tdx-reference-ramdisk-image"
INITRAMFS_IMAGE_BUNDLE ?= "0"

# use the FIT image with the ramdisk
KERNEL_IMAGE_NAME:prepend = "${INITRAMFS_IMAGE_NAME}-"
