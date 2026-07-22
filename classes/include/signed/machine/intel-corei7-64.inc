IMAGE_EFI_BOOT_FILES:append = " ext4_x64.signed.efi;EFI/systemd/drivers/ext4_x64.efi"
do_image_wic[depends] += " edk2-ext4dxe:do_deploy"
