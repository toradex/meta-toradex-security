SRC_URI:append = "\
    file://0001-binman-k3-add-PKCS-11-HSM-signing-support.patch \
"

update_signing_keys() {
    if [ "${TDX_SIGNED_HSM}" = "0" ]; then
        echo "Updating signing keys directory '${S}/arch/arm/mach-k3/keys'"
        rm -Rf "${S}/arch/arm/mach-k3/keys"
        cp -rv "${TDX_K3_SECBOOT_KEY_DIR}" "${S}/arch/arm/mach-k3/keys"
    fi
}

do_unpack:append() {
    bb.build.exec_func('update_signing_keys', d)
}

update_key_name() {
    config=${1}
    sed -i '/^CONFIG_SYS_K3_KEY=/d' "${config}"
    echo "CONFIG_SYS_K3_KEY=\"${TDX_K3_SECBOOT_KEY_NAME}\"" >> "${config}"
}

do_configure:append() {
    if [ -n "${UBOOT_CONFIG}" ]; then
        for config in ${UBOOT_MACHINE}; do
            update_key_name ${B}/${config}/.config
        done
    else
        update_key_name ${B}/.config
    fi
}

require ${@oe.utils.conditional('TDX_SIGNED_HSM', '1', 'u-boot-k3-secboot-hsm.inc', '', d)}
