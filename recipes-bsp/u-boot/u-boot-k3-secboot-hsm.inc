inherit tdx-signed-hsm-env

SRC_URI += "\
    file://0001-binman-x509_cert-support-PKCS-11-URI-in-keyfile-for-.patch \
"

do_compile:prepend() {
    tdx_signed_hsm_vars_export_k3_binman
}
