inherit tdx-signed-hsm-env

do_compile:prepend() {
    tdx_signed_hsm_vars_export
}
