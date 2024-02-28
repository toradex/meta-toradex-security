SUMMARY = "Toradex Embedded Linux Reference Ramdisk Image"
DESCRIPTION = "Minimal ramdisk image to boot a dm-verity based rootfs"

LICENSE = "MIT"

# Some Toradex machines append a value to IMAGE_BASENAME (e.g. -upstream
# is appended on Colibri iMX6). This currently causes an issue when the
# ramdisk image is deployed, because IMAGE_BASENAME != INITRAMFS_IMAGE_NAME.
# So let's use an anonymous function to forcibly set IMAGE_BASENAME
# and avoid any appends to it
export IMAGE_BASENAME
python (){
    d.setVar('IMAGE_BASENAME', 'tdx-reference-ramdisk-image')
}

IMAGE_NAME_SUFFIX ?= ""
IMAGE_LINGUAS = ""

INITRAMFS_SCRIPTS ?= "\
    initramfs-framework-base \
    initramfs-module-udev \
"

PACKAGE_INSTALL = "\
    ${INITRAMFS_SCRIPTS} \
    ${VIRTUAL-RUNTIME_base-utils} \
    udev \
"

IMAGE_FEATURES = ""

# avoid any circular dependencies
DEPENDS:remove = "\
    u-boot-default-script \
    virtual/bootloader \
    imx-boot \
    tezi-metadata \
    virtual/dtb \
"

IMAGE_FSTYPES = "${INITRAMFS_FSTYPES}"
IMAGE_FSTYPES:remove = "teziimg"

inherit core-image

IMAGE_ROOTFS_SIZE = "8192"
IMAGE_ROOTFS_EXTRA_SPACE = "0"
