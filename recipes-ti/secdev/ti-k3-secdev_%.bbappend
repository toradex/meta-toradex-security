update_signing_keys() {
    rm -Rf ${S}/keys
    ln -s ${TDX_K3_HSSE_KEY_DIR} ${S}/keys
}

do_unpack:append() {
    bb.build.exec_func('update_signing_keys', d)    
}
