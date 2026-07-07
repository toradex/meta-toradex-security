UEFI_SB_NATIVE_TASK_DEPS = "\
    efitools-native:do_populate_sysroot \
    openssl-native:do_populate_sysroot \
    sbsigntool-native:do_populate_sysroot \
    util-linux-native:do_populate_sysroot \
    "

UEFI_SB_GENERATE_KEYS ?= "1"
UEFI_SB_SIGN_ENABLE ?= "1"
UEFI_SB_KEYDIR ?= "${TOPDIR}/keys/uefi"

uefi_sb_check_artifacts() {
    missing_artifacts=""
    missing_artifacts_count=0
    existing_artifacts_count=0

    for artifact in \
        PK.key PK.crt PK.esl PK.auth \
        KEK.key KEK.crt KEK.esl KEK.auth \
        db.key db.crt db.esl db.auth \
        dbx.esl dbx.auth; do
        if [ -f "${UEFI_SB_KEYDIR}/${artifact}" ]; then
            existing_artifacts_count=$(expr ${existing_artifacts_count} + 1)
        else
            missing_artifacts_count=$(expr ${missing_artifacts_count} + 1)
            missing_artifacts="${missing_artifacts} ${artifact}"
        fi
    done
}

uefi_sb_generate_dev_keys() {
    guid="$(uuidgen --random)"

    bbnote "Generating development UEFI Secure Boot keys in ${UEFI_SB_KEYDIR}"
    bbnote "Using UEFI signature owner GUID ${guid}"

    mkdir -p "${UEFI_SB_KEYDIR}"

    "${STAGING_BINDIR_NATIVE}/openssl" req -newkey rsa:4096 -nodes -new -x509 -sha256 -days 7300 \
        -subj "/CN=Torizon PK/" \
        -keyout "${UEFI_SB_KEYDIR}/PK.key" \
        -out "${UEFI_SB_KEYDIR}/PK.crt"
    "${STAGING_BINDIR_NATIVE}/openssl" req -newkey rsa:4096 -nodes -new -x509 -sha256 -days 7300 \
        -subj "/CN=Torizon KEK/" \
        -keyout "${UEFI_SB_KEYDIR}/KEK.key" \
        -out "${UEFI_SB_KEYDIR}/KEK.crt"
    "${STAGING_BINDIR_NATIVE}/openssl" req -newkey rsa:4096 -nodes -new -x509 -sha256 -days 7300 \
        -subj "/CN=Torizon db/" \
        -keyout "${UEFI_SB_KEYDIR}/db.key" \
        -out "${UEFI_SB_KEYDIR}/db.crt"

    "${STAGING_BINDIR_NATIVE}/cert-to-efi-sig-list" -g "${guid}" \
        "${UEFI_SB_KEYDIR}/PK.crt" "${UEFI_SB_KEYDIR}/PK.esl"
    "${STAGING_BINDIR_NATIVE}/cert-to-efi-sig-list" -g "${guid}" \
        "${UEFI_SB_KEYDIR}/KEK.crt" "${UEFI_SB_KEYDIR}/KEK.esl"
    "${STAGING_BINDIR_NATIVE}/cert-to-efi-sig-list" -g "${guid}" \
        "${UEFI_SB_KEYDIR}/db.crt" "${UEFI_SB_KEYDIR}/db.esl"
    : > "${UEFI_SB_KEYDIR}/dbx.esl"

    "${STAGING_BINDIR_NATIVE}/sign-efi-sig-list" -g "${guid}" \
        -k "${UEFI_SB_KEYDIR}/PK.key" -c "${UEFI_SB_KEYDIR}/PK.crt" \
        PK "${UEFI_SB_KEYDIR}/PK.esl" "${UEFI_SB_KEYDIR}/PK.auth"
    "${STAGING_BINDIR_NATIVE}/sign-efi-sig-list" -g "${guid}" \
        -k "${UEFI_SB_KEYDIR}/PK.key" -c "${UEFI_SB_KEYDIR}/PK.crt" \
        KEK "${UEFI_SB_KEYDIR}/KEK.esl" "${UEFI_SB_KEYDIR}/KEK.auth"
    "${STAGING_BINDIR_NATIVE}/sign-efi-sig-list" -g "${guid}" \
        -k "${UEFI_SB_KEYDIR}/KEK.key" -c "${UEFI_SB_KEYDIR}/KEK.crt" \
        db "${UEFI_SB_KEYDIR}/db.esl" "${UEFI_SB_KEYDIR}/db.auth"
    "${STAGING_BINDIR_NATIVE}/sign-efi-sig-list" -g "${guid}" \
        -k "${UEFI_SB_KEYDIR}/KEK.key" -c "${UEFI_SB_KEYDIR}/KEK.crt" \
        dbx "${UEFI_SB_KEYDIR}/dbx.esl" "${UEFI_SB_KEYDIR}/dbx.auth"
}

uefi_sb_prepare_keys() {
    if [ -z "${UEFI_SB_GENERATE_KEYS}" ] || [ -z "${UEFI_SB_KEYDIR}" ]; then
        bbfatal "UEFI Secure Boot key generation is not correctly configured."
    fi

    uefi_sb_check_artifacts

    if [ "${missing_artifacts_count}" -eq 0 ]; then
        bbnote "Complete UEFI Secure Boot key set found in ${UEFI_SB_KEYDIR}"
        return
    fi

    if [ "${existing_artifacts_count}" -gt 0 ]; then
        bbfatal "Incomplete UEFI Secure Boot key set in ${UEFI_SB_KEYDIR}. Missing:${missing_artifacts}"
    fi

    if [ "${UEFI_SB_GENERATE_KEYS}" != "1" ]; then
        bbfatal "UEFI_SB_GENERATE_KEYS is disabled and no complete key set was found in ${UEFI_SB_KEYDIR}. Import production keys before building."
    fi

    uefi_sb_generate_dev_keys
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
