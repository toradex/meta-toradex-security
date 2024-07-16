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
    initramfs-module-dmverity \
"

PACKAGE_INSTALL = "\
    ${INITRAMFS_SCRIPTS} \
    ${VIRTUAL-RUNTIME_base-utils} \
    udev \
    cryptsetup \
"

# mount the rootfs image only from a dm-verity image
BAD_RECOMMENDATIONS += "initramfs-module-rootfs"

IMAGE_FEATURES = ""

# avoid any circular dependencies
DEPENDS:remove = "\
    u-boot-default-script \
    virtual/bootloader \
    imx-boot \
    tezi-metadata \
    virtual/dtb \
"

# rootfs should be built before the ramdisk so we have
# dm-verity.env to add to the ramdisk
do_rootfs[depends] += "${DM_VERITY_IMAGE}:do_image_${DM_VERITY_IMAGE_TYPE}"

# ensure dm-verity.env is updated also when rebuilding DM_VERITY_IMAGE
do_image[nostamp] = "1"

IMAGE_FSTYPES = "${INITRAMFS_FSTYPES}"
IMAGE_FSTYPES:remove = "teziimg"

inherit core-image

IMAGE_ROOTFS_SIZE = "8192"
IMAGE_ROOTFS_EXTRA_SPACE = "0"

# deploy verity hash into ramdisk image
deploy_verity_hash() {
    install -D -m 0644 \
        ${STAGING_VERITY_DIR}/*.${DM_VERITY_IMAGE_TYPE}.verity.env \
        ${IMAGE_ROOTFS}${datadir}/misc/dm-verity.env
}
IMAGE_PREPROCESS_COMMAND += "deploy_verity_hash;"
