require ${@ 'classes/include/signed/tdx-signed-uefi-secboot.inc' if 'tdx-signed' in d.getVar('OVERRIDES').split(':') else ''}

UEFI_SB_NATIVE_TASK_DEPS ?= ""
do_configure[depends] += "${UEFI_SB_NATIVE_TASK_DEPS}"

do_deploy:append:tdx-signed() {
    uefi_sb_prepare_keys

    install -d "${DEPLOYDIR}/loader/keys/auto"
    for auth in PK KEK db dbx; do
        install -m 0644 "${UEFI_SB_KEYDIR}/${auth}.auth" \
            "${DEPLOYDIR}/loader/keys/auto/${auth}.auth"
    done
}
