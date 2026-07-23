UEFI_SB_NATIVE_TASK_DEPS = "\
    efitools-native:do_populate_sysroot \
    sbsigntool-native:do_populate_sysroot \
    "

UEFI_SB_SIGN_ENABLE ?= "1"
UEFI_SB_DEV_KEYDIR = "${META_TORADEX_SECURITY_LAYERDIR}/keys/uefi-dev"
UEFI_SB_KEYDIR ?= "${UEFI_SB_DEV_KEYDIR}"
UEFI_SB_KEY_FILES = "\
    PK.key PK.crt PK.esl PK.auth \
    KEK.key KEK.crt KEK.esl KEK.auth \
    db.key db.crt db.esl db.auth \
    dbx.esl dbx.auth \
    "
UEFI_SB_KEY_FILE_CHECKSUMS = "${@' '.join('%s/%s:True' % (d.getVar('UEFI_SB_KEYDIR'), key) for key in d.getVar('UEFI_SB_KEY_FILES').split())}"

uefi_sb_check_artifacts() {
    missing_artifacts=""
    missing_artifacts_count=0

    for artifact in ${UEFI_SB_KEY_FILES}; do
        if [ ! -f "${UEFI_SB_KEYDIR}/${artifact}" ]; then
            missing_artifacts_count=$(expr ${missing_artifacts_count} + 1)
            missing_artifacts="${missing_artifacts} ${artifact}"
        fi
    done
}

uefi_sb_warn_dev_keys() {
    if [ "${UEFI_SB_KEYDIR}" = "${UEFI_SB_DEV_KEYDIR}" ] && [ -z "${uefi_sb_dev_key_warning_emitted}" ]; then
        bbwarn "UEFI Secure Boot development keys from ${UEFI_SB_DEV_KEYDIR} are being used. These public test credentials must not sign production images. Set UEFI_SB_KEYDIR to production keys."
        uefi_sb_dev_key_warning_emitted=1
    fi
}

uefi_sb_prepare_keys() {
    if [ -z "${UEFI_SB_KEYDIR}" ]; then
        bbfatal "UEFI Secure Boot key directory is not configured."
    fi

    uefi_sb_check_artifacts

    if [ "${missing_artifacts_count}" -ne 0 ]; then
        bbfatal "Incomplete UEFI Secure Boot key set in ${UEFI_SB_KEYDIR}. Missing:${missing_artifacts}"
    fi

    uefi_sb_warn_dev_keys
}

uefi_sb_sign() {
    input="$1"
    output="$2"

    if [ "${UEFI_SB_SIGN_ENABLE}" != "1" ]; then
        bbnote "UEFI Secure Boot signing disabled; using unsigned EFI binary"
        if [ "${input}" != "${output}" ]; then
            cp -f "${input}" "${output}"
        fi
        return
    fi

    uefi_sb_prepare_keys

    "${STAGING_BINDIR_NATIVE}/sbsign" \
        --key "${UEFI_SB_KEYDIR}/db.key" \
        --cert "${UEFI_SB_KEYDIR}/db.crt" \
        --output "${output}" \
        "${input}"
}
