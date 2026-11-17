UKI_FILENAME = "torizon.efi"

UKI_CMDLINE = "${OSTREE_KERNEL_ARGS}"

UKI_SB_KEY = "${@d.getVar('UEFI_SB_UKI_KEYDIR') + '/' + d.getVar('UEFI_SB_UKI_KEY_NAME') + '.key' if d.getVar('UEFI_SB_SIGN_ENABLE') == '1' else ''}"
UKI_SB_CERT = "${@d.getVar('UEFI_SB_UKI_KEYDIR') + '/' + d.getVar('UEFI_SB_UKI_KEY_NAME') + '.crt' if d.getVar('UEFI_SB_SIGN_ENABLE') == '1' else ''}"

do_uki[depends] += "${@ 'sbsigntool-native:do_populate_sysroot' if d.getVar('UEFI_SB_SIGN_ENABLE') == '1' else ''}"
