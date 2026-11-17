UEFI_SB_NATIVE_TASK_DEPS = "\
    efitools-native:do_populate_sysroot \
    openssl-native:do_populate_sysroot \
    perl-native:do_populate_sysroot \
    sbsigntool-native:do_populate_sysroot \
    "

UEFI_SB_SIGN_ENABLE ?= "1"
UEFI_SB_DEV_KEYDIR = "${META_TORADEX_SECURITY_LAYERDIR}/keys/uefi-dev"
UEFI_SB_KEYDIR ?= "${UEFI_SB_DEV_KEYDIR}"
UEFI_SB_BOOT_KEYDIR ?= "${UEFI_SB_KEYDIR}"
UEFI_SB_UKI_KEYDIR ?= "${UEFI_SB_KEYDIR}"
UEFI_SB_BOOT_KEY_NAME ?= "key-boot"
UEFI_SB_UKI_KEY_NAME ?= "key-uki"
UEFI_SB_ENROLL_KEY_FILES = "\
    PK.key PK.crt PK.esl PK.auth \
    KEK.key KEK.crt KEK.esl KEK.auth \
    db.esl db.auth \
    dbx.esl dbx.auth \
    "
UEFI_SB_BOOT_KEY_FILES = "${UEFI_SB_BOOT_KEY_NAME}.key ${UEFI_SB_BOOT_KEY_NAME}.crt"
UEFI_SB_UKI_KEY_FILES = "${UEFI_SB_UKI_KEY_NAME}.key ${UEFI_SB_UKI_KEY_NAME}.crt"
UEFI_SB_KEY_FILE_CHECKSUMS = "${@' '.join('%s/%s:True' % (keydir, key) for keydir, files in ((d.getVar('UEFI_SB_KEYDIR'), d.getVar('UEFI_SB_ENROLL_KEY_FILES')), (d.getVar('UEFI_SB_BOOT_KEYDIR'), d.getVar('UEFI_SB_BOOT_KEY_FILES')), (d.getVar('UEFI_SB_UKI_KEYDIR'), d.getVar('UEFI_SB_UKI_KEY_FILES'))) for key in files.split())}"

# Incremental UEFI variable updates. These inputs are separate from the
# manufacturing enrollment artifacts above and must never overwrite them.
UEFI_SB_DB_ADD_CERTS ?= ""
UEFI_SB_DBX_ADD_HASHES ?= ""
UEFI_SB_DBX_ADD_CERTS ?= ""
UEFI_SB_UPDATE_OWNER_GUID ?= "6f356f82-8ce1-40ed-8820-7471698105f3"
UEFI_SB_UPDATE_DEPLOYDIR ?= "${DEPLOYDIR}/uefi-sb-updates"
UEFI_SB_UPDATE_INPUT_FILE_CHECKSUMS = "${@' '.join('%s:True' % path for path in (d.getVar('UEFI_SB_DB_ADD_CERTS') + ' ' + d.getVar('UEFI_SB_DBX_ADD_CERTS')).split())}"

uefi_sb_check_artifacts() {
    missing_artifacts=""
    missing_artifacts_count=0

    for keydir_and_files in \
        "${UEFI_SB_KEYDIR}|${UEFI_SB_ENROLL_KEY_FILES}" \
        "${UEFI_SB_BOOT_KEYDIR}|${UEFI_SB_BOOT_KEY_FILES}" \
        "${UEFI_SB_UKI_KEYDIR}|${UEFI_SB_UKI_KEY_FILES}"; do
        keydir=${keydir_and_files%%|*}
        files=${keydir_and_files#*|}
        for artifact in ${files}; do
            if [ ! -f "${keydir}/${artifact}" ]; then
                missing_artifacts_count=$(expr ${missing_artifacts_count} + 1)
                missing_artifacts="${missing_artifacts} ${keydir}/${artifact}"
            fi
        done
    done
}

uefi_sb_warn_dev_keys() {
    if { [ "${UEFI_SB_KEYDIR}" = "${UEFI_SB_DEV_KEYDIR}" ] || \
         [ "${UEFI_SB_BOOT_KEYDIR}" = "${UEFI_SB_DEV_KEYDIR}" ] || \
         [ "${UEFI_SB_UKI_KEYDIR}" = "${UEFI_SB_DEV_KEYDIR}" ]; } && \
       [ -z "${uefi_sb_dev_key_warning_emitted}" ]; then
        bbwarn "UEFI Secure Boot development keys from ${UEFI_SB_DEV_KEYDIR} are being used. These public test credentials must not sign production images. Set UEFI_SB_KEYDIR to production keys."
        uefi_sb_dev_key_warning_emitted=1
    fi
}

uefi_sb_prepare_keys() {
    if [ -z "${UEFI_SB_KEYDIR}" ] || [ -z "${UEFI_SB_BOOT_KEYDIR}" ] || [ -z "${UEFI_SB_UKI_KEYDIR}" ]; then
        bbfatal "UEFI Secure Boot key directory is not configured."
    fi

    uefi_sb_check_artifacts

    if [ "${missing_artifacts_count}" -ne 0 ]; then
        bbfatal "Incomplete UEFI Secure Boot key set. Missing:${missing_artifacts}"
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
        --key "${UEFI_SB_BOOT_KEYDIR}/${UEFI_SB_BOOT_KEY_NAME}.key" \
        --cert "${UEFI_SB_BOOT_KEYDIR}/${UEFI_SB_BOOT_KEY_NAME}.crt" \
        --output "${output}" \
        "${input}"
}

uefi_sb_write_update_manifest() {
    variable="$1"
    source_file="$2"
    source_sha256="$3"
    artifact="$4"
    authenticode_sha256="${5:-}"
    payload_sha256=$(sha256sum "${artifact}" | awk '{print $1}')

    printf '%s\n' \
        '{' \
        "  \"hardware_id\": \"uefi-${variable}-append\"," \
        "  \"uefi_variable\": \"${variable}\"," \
        "  \"build_version\": \"${DISTRO_VERSION}\"," \
        "  \"source\": \"$(basename "${source_file}")\"," \
        "  \"source_sha256\": \"${source_sha256}\"," \
        "  \"authenticode_sha256\": \"${authenticode_sha256}\"," \
        "  \"payload_sha256\": \"${payload_sha256}\"," \
        '  "test_only": false' \
        '}' > "${artifact}.json"
}

uefi_sb_make_update_auth() {
    [ -n "${UEFI_SB_DB_ADD_CERTS}${UEFI_SB_DBX_ADD_HASHES}${UEFI_SB_DBX_ADD_CERTS}" ] || return 0

    uefi_sb_prepare_keys
    install -d "${UEFI_SB_UPDATE_DEPLOYDIR}"
    workdir=$(mktemp -d "${T}/uefi-sb-update.XXXXXX")

    for cert in ${UEFI_SB_DB_ADD_CERTS}; do
        [ -f "${cert}" ] || bbfatal "UEFI DB update certificate not found: ${cert}"
        source_sha256=$(sha256sum "${cert}" | awk '{print $1}')
        artifact="${UEFI_SB_UPDATE_DEPLOYDIR}/db-append-${DISTRO_VERSION}-${source_sha256}.auth"

        "${STAGING_BINDIR_NATIVE}/cert-to-efi-sig-list" \
            -g "${UEFI_SB_UPDATE_OWNER_GUID}" "${cert}" "${workdir}/db.esl"
        "${STAGING_BINDIR_NATIVE}/sign-efi-sig-list" \
            -a -g "${UEFI_SB_UPDATE_OWNER_GUID}" \
            -k "${UEFI_SB_KEYDIR}/KEK.key" -c "${UEFI_SB_KEYDIR}/KEK.crt" \
            db "${workdir}/db.esl" "${artifact}"
        uefi_sb_write_update_manifest db "${cert}" "${source_sha256}" "${artifact}"
    done

    for authenticode_sha256 in ${UEFI_SB_DBX_ADD_HASHES}; do
        printf '%s' "${authenticode_sha256}" | grep -Eq '^[0-9a-fA-F]{64}$' || \
            bbfatal "UEFI DBX hash must be 64 hexadecimal characters: ${authenticode_sha256}"
        authenticode_sha256=$(printf '%s' "${authenticode_sha256}" | tr 'A-F' 'a-f')
        artifact="${UEFI_SB_UPDATE_DEPLOYDIR}/dbx-append-${DISTRO_VERSION}-${authenticode_sha256}.auth"

        python3 -c 'import sys, struct, uuid
digest, owner, out = sys.argv[1:4]
EFI_CERT_SHA256_GUID = uuid.UUID("c1c41626-504c-4092-aca9-41f936934328")
with open(out, "wb") as fh:
    fh.write(EFI_CERT_SHA256_GUID.bytes_le + struct.pack("<3I", 76, 0, 48)
             + uuid.UUID(owner).bytes_le + bytes.fromhex(digest))
' "${authenticode_sha256}" "${UEFI_SB_UPDATE_OWNER_GUID}" "${workdir}/dbx.esl"

        "${STAGING_BINDIR_NATIVE}/sign-efi-sig-list" \
            -a -g "${UEFI_SB_UPDATE_OWNER_GUID}" \
            -k "${UEFI_SB_KEYDIR}/KEK.key" -c "${UEFI_SB_KEYDIR}/KEK.crt" \
            dbx "${workdir}/dbx.esl" "${artifact}"
        uefi_sb_write_update_manifest dbx "${authenticode_sha256}" \
            "${authenticode_sha256}" "${artifact}" "${authenticode_sha256}"
    done

    for cert in ${UEFI_SB_DBX_ADD_CERTS}; do
        [ -f "${cert}" ] || bbfatal "UEFI DBX update certificate not found: ${cert}"
        openssl x509 -in "${cert}" -outform DER -out "${workdir}/dbx-cert.der" || \
            bbfatal "UEFI DBX update certificate is not a valid X.509 certificate: ${cert}"
        cert_sha256=$(sha256sum "${workdir}/dbx-cert.der" | awk '{print $1}')
        for active_cert in \
            "${UEFI_SB_BOOT_KEYDIR}/${UEFI_SB_BOOT_KEY_NAME}.crt" \
            "${UEFI_SB_UKI_KEYDIR}/${UEFI_SB_UKI_KEY_NAME}.crt"; do
            openssl x509 -in "${active_cert}" -outform DER -out "${workdir}/active-cert.der" || \
                bbfatal "Active UEFI signing certificate is not a valid X.509 certificate: ${active_cert}"
            active_cert_sha256=$(sha256sum "${workdir}/active-cert.der" | awk '{print $1}')
            [ "${cert_sha256}" != "${active_cert_sha256}" ] || \
                bbfatal "Refusing to revoke active UEFI signing certificate: ${cert}"
        done
        artifact="${UEFI_SB_UPDATE_DEPLOYDIR}/dbx-cert-append-${DISTRO_VERSION}-${cert_sha256}.auth"

        "${STAGING_BINDIR_NATIVE}/cert-to-efi-sig-list" \
            -g "${UEFI_SB_UPDATE_OWNER_GUID}" "${cert}" "${workdir}/dbx.esl"
        "${STAGING_BINDIR_NATIVE}/sign-efi-sig-list" \
            -a -g "${UEFI_SB_UPDATE_OWNER_GUID}" \
            -k "${UEFI_SB_KEYDIR}/KEK.key" -c "${UEFI_SB_KEYDIR}/KEK.crt" \
            dbx "${workdir}/dbx.esl" "${artifact}"
        uefi_sb_write_update_manifest dbx "${cert}" "${cert_sha256}" "${artifact}"
    done

    rm -rf "${workdir}"
}

IMAGE_INSTALL:append:intel-corei7-64:tdx-signed = " uefi-sb-update"
