require classes/include/signed/tdx-signed-uki.inc

IMAGE_CLASSES:append = " uki"
IMAGE_CLASSES:remove:pn-initramfs-ostree-torizon-image = "uki"

IMAGE_EFI_BOOT_FILES:append = " ext4_x64.signed.efi;EFI/systemd/drivers/ext4_x64.efi"
do_image_wic[depends] += " edk2-ext4dxe:do_deploy"
