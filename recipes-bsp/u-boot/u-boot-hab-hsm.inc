inherit tdx-signed-hsm-env

# binman signs the i.MX8M boot container during do_compile
do_compile:prepend:mx8m-generic-bsp() {
    tdx_signed_hsm_vars_export
}
