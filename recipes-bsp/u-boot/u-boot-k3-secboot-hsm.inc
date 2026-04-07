inherit tdx-signed-hsm-env

do_compile:prepend() {
    tdx_signed_hsm_vars_export
}
do_compile[lockfiles] = "${TMPDIR}/locks/hsm.lock"

do_deploy:prepend() {
    tdx_signed_hsm_vars_export
}
do_deploy[lockfiles] = "${TMPDIR}/locks/hsm.lock"
