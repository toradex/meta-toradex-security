require ${@ 'classes/include/signed/tdx-signed-uefi-secboot.inc' if 'tdx-signed' in d.getVar('OVERRIDES').split(':') else ''}

UEFI_SB_NATIVE_TASK_DEPS ?= ""
UEFI_SB_KEY_FILE_CHECKSUMS ?= ""
do_deploy[depends] += "${UEFI_SB_NATIVE_TASK_DEPS}"
do_deploy[file-checksums] += "${UEFI_SB_KEY_FILE_CHECKSUMS}"

do_deploy:append:intel-corei7-64:tdx-signed() {
    uefi_sb_prepare_keys

    uefi_sb_sign "${DEPLOYDIR}/ext4_x64.efi" \
        "${DEPLOYDIR}/ext4_x64.signed.efi"
}
