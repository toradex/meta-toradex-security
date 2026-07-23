require classes/include/signed/tdx-signed-uefi-secboot.inc

OVMF_SB_OWNER_GUID ??= "a5c059a1-94e4-4aa7-87b5-ab155c2bf072"
OVMF_SB_VARS ??= "ovmf.secboot.vars"

OVMF_SB_VARS_DEPENDS = "${@bb.utils.contains('PACKAGECONFIG', 'secureboot', 'python3-virt-firmware-native:do_populate_sysroot ${UEFI_SB_NATIVE_TASK_DEPS}', '', d)}"
do_deploy[depends] += "${OVMF_SB_VARS_DEPENDS}"
do_deploy[file-checksums] += "${UEFI_SB_KEY_FILE_CHECKSUMS}"

DEPLOYDEP:append:class-target = " ${OVMF_SB_VARS_DEPENDS}"

ovmf_generate_secureboot_vars() {
    if ! ${@bb.utils.contains('PACKAGECONFIG', 'secureboot', 'true', 'false', d)}; then
        return
    fi

    uefi_sb_prepare_keys

    CRYPTOGRAPHY_OPENSSL_NO_LEGACY=1 "${STAGING_BINDIR_NATIVE}/virt-fw-vars" \
        --input "${WORKDIR}/ovmf/ovmf.vars.fd" \
        --output "${WORKDIR}/ovmf/${OVMF_SB_VARS}.fd" \
        --set-pk "${OVMF_SB_OWNER_GUID}" "${UEFI_SB_KEYDIR}/PK.crt" \
        --add-kek "${OVMF_SB_OWNER_GUID}" "${UEFI_SB_KEYDIR}/KEK.crt" \
        --add-db "${OVMF_SB_OWNER_GUID}" "${UEFI_SB_KEYDIR}/db.crt" \
        --secure-boot
}

do_deploy:append:class-target() {
    if ! ${@bb.utils.contains('PACKAGECONFIG', 'secureboot', 'true', 'false', d)}; then
        return
    fi

    ovmf_generate_secureboot_vars

    qemu-img convert -f raw -O qcow2 \
        "${WORKDIR}/ovmf/${OVMF_SB_VARS}.fd" \
        "${DEPLOYDIR}/${OVMF_SB_VARS}.qcow2"
}
