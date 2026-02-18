inherit tdx-signed-hsm-env

do_assemble_fitimage_initramfs:prepend() {
    tdx_signed_hsm_vars_export
}

do_assemble_fitimage:prepend() {
    tdx_signed_hsm_vars_export
}
