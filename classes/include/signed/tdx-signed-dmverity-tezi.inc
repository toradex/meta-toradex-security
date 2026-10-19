IMAGE_FSTYPES:remove = "wic.bmap wic.gz"
DEPENDS:remove:pn-${DM_VERITY_IMAGE} = "imx-boot"
IMAGE_BOOTFS_DEPENDS:pn-${DM_VERITY_IMAGE} = "virtual/kernel:do_build"
IMAGE_BOOTFS_DEPENDS:mx8-generic-bsp:pn-${DM_VERITY_IMAGE} = "${@ 'imx-boot:do_build' if d.getVar('UBOOT_PROVIDES_BOOT_CONTAINER') == '0' else 'virtual/kernel:do_build'}"
IMAGE_BOOTFS_DEPENDS:mx9-generic-bsp:pn-${DM_VERITY_IMAGE} = "${@ 'imx-boot:do_build' if d.getVar('UBOOT_PROVIDES_BOOT_CONTAINER') == '0' else 'virtual/kernel:do_build'}"
do_image_bootfs[depends] += "${IMAGE_BOOTFS_DEPENDS}"

# make sure the kernel FIT image with the ramdisk is deployed before generating the Tezi image
do_image_bootfs[depends] += "virtual/kernel:do_deploy"
