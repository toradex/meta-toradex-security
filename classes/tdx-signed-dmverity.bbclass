# add the class name in the overrides for conditional assignment
DISTROOVERRIDES .= ":tdx-signed-dmverity"

# a ramdisk is required to check the root hash of the verity image
INITRAMFS_IMAGE ?= "tdx-reference-ramdisk-image"
INITRAMFS_IMAGE_BUNDLE ?= "0"

# use the FIT image with the ramdisk
KERNEL_IMAGE_NAME:prepend = "${INITRAMFS_IMAGE_NAME}-"

# rootfs image that will be used to create dm-verity image
# override it when building a different image recipe
DM_VERITY_IMAGE ?= "tdx-reference-minimal-image"
DM_VERITY_IMAGE_TYPE ?= "ext4"

# enable creation of dm-verity image
IMAGE_CLASSES += "dm-verity-img"

# we don't need all features from meta-security, so we don't
# include 'security' in DISTROFEATURES. Because of that, a
# warning message is displayed by meta-security. Avoid that by
# setting the variable below.
SKIP_META_SECURITY_SANITY_CHECK = "1"

# Tezi configuration
TEZI_ROOT_SUFFIX = "ext4.verity"
TEZI_ROOT_FSTYPE = "raw"

# Easy Installer needs the size of the rootfs image so it can add to image.json
CONVERSION_DEPENDS_verity:append = " bc-native"
verity_setup:append() {
    SIZE_IN_KB=$(echo "${SIZE}/1024" | bc)
    echo ${SIZE_IN_KB} > ${T}/image-size.${TEZI_ROOT_NAME}
}

# fix cyclic dependency when inheriting image_type_tezi.bbclass
IMAGE_FSTYPES:remove = "wic.bmap wic.gz"
DEPENDS:remove:pn-${DM_VERITY_IMAGE} = "imx-boot"
IMAGE_BOOTFS_DEPENDS = "${@ 'virtual/kernel:do_build' if 'pn-${DM_VERITY_IMAGE}' in d.getVar('OVERRIDES').split(':') else ''}"
IMAGE_BOOTFS_DEPENDS:mx8-generic-bsp = "${@ 'imx-boot:do_build' if 'pn-${DM_VERITY_IMAGE}' in d.getVar('OVERRIDES').split(':') else ''}"
do_image_bootfs[depends] += "${IMAGE_BOOTFS_DEPENDS}"
