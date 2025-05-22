update_signing_keys() {
    echo "Updating signing keys directory '${S}/keys'"
    rm -Rf "${S}/keys"
    cp -rv ${TDX_K3_SECBOOT_KEY_DIR} "${S}/keys"
}

do_unpack:append() {
    bb.build.exec_func('update_signing_keys', d)
}
