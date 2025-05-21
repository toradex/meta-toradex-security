update_signing_keys() {
    echo "Updating signing keys directory '${S}/arch/arm/mach-k3/keys'"
    rm -Rf "${S}/arch/arm/mach-k3/keys"
    cp -rv "${TDX_K3_SECBOOT_KEY_DIR}" "${S}/arch/arm/mach-k3/keys"
}

do_unpack:append() {
    bb.build.exec_func('update_signing_keys', d)
}
