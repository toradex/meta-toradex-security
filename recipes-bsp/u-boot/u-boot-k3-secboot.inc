update_signing_keys() {
    if [ "${TDX_SIGNED_HSM}" = "1" ]; then
        echo "HSM-backed signing: skipping copy of on-disk signing keys"
        return 0
    else
        echo "Updating signing keys directory '${S}/arch/arm/mach-k3/keys'"
        rm -Rf "${S}/arch/arm/mach-k3/keys"
        cp -rv "${TDX_K3_SECBOOT_KEY_DIR}" "${S}/arch/arm/mach-k3/keys"
    fi
}

do_unpack:append() {
    bb.build.exec_func('update_signing_keys', d)
}

# enable HSM-backed signing for the K3 boot container
require ${@oe.utils.conditional('TDX_SIGNED_HSM', '1', 'u-boot-k3-secboot-hsm.inc', '', d)}
