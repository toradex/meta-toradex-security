require ${@ 'classes/include/signed/tdx-signed-uefi-secboot.inc' if 'tdx-signed' in d.getVar('OVERRIDES').split(':') else ''}

FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"
SRC_URI:append:tdx-signed = " file://loader.conf"

UEFI_SB_NATIVE_TASK_DEPS ?= ""
UEFI_SB_KEY_FILE_CHECKSUMS ?= ""
do_deploy[depends] += "${UEFI_SB_NATIVE_TASK_DEPS}"
do_deploy[file-checksums] += "${UEFI_SB_KEY_FILE_CHECKSUMS}"

do_deploy:append:tdx-signed() {
    uefi_sb_prepare_keys

    install -d "${DEPLOYDIR}/loader/keys/auto"
    for auth in PK KEK db dbx; do
        install -m 0644 "${UEFI_SB_KEYDIR}/${auth}.auth" \
            "${DEPLOYDIR}/loader/keys/auto/${auth}.auth"
    done

    uefi_sb_sign "${DEPLOYDIR}/systemd-bootx64.efi" \
        "${DEPLOYDIR}/systemd-bootx64.signed.efi"

    install -m 0644 "${WORKDIR}/loader.conf" "${DEPLOYDIR}/loader/loader.conf"
}
