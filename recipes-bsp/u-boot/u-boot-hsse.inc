update_signing_keys() {
    rm -Rf ${S}/board/ti/keys
    ln -s ${TDX_K3_HSSE_KEY_DIR} ${S}/board/ti/keys
}

do_unpack:append() {
    bb.build.exec_func('update_signing_keys', d)    
}
