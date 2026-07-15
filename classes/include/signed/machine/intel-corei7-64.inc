require classes/include/signed/tdx-signed-uki.inc

WKS_FILE:intel-corei7-64:sota = "efiimage-sota-signed.wks"

OSTREE_BOOTLOADER:intel-corei7-64 = "systemd-boot"
EFI_PROVIDER:intel-corei7-64 = "systemd-boot"

IMAGE_CLASSES:append = " uki"
IMAGE_CLASSES:remove:pn-initramfs-ostree-torizon-image = "uki"
IMAGE_INSTALL:remove = "grub-ota-fallback"

# Enable OVMF Secure Boot build (produces ovmf.secboot.fd + EnrollDefaultKeys.efi)
PACKAGECONFIG:append:pn-ovmf = " secureboot"
# Expose TPM2 to OVMF (auto-enables -D TPM_ENABLE=TRUE in the OVMF build)
MACHINE_FEATURES:append = " tpm2"

PACKAGECONFIG:append:pn-systemd = " efi"

IMAGE_EFI_BOOT_FILES = " \
    ext4_x64.signed.efi;EFI/systemd/drivers/ext4_x64.efi \
    loader/keys/auto/db.auth \
    loader/keys/auto/dbx.auth \
    loader/keys/auto/KEK.auth \
    loader/keys/auto/KEK.auth \
    loader/keys/auto/PK.auth \
    loader/keys/auto/PK.auth \
    loader/loader.conf;loader/loader.conf \
    systemd-bootx64.signed.efi;EFI/BOOT/bootx64.efi \
    "

do_image_wic[depends] += " edk2-ext4dxe:do_deploy"
