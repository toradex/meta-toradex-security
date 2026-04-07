inherit tdx-signed-hsm-env

do_assemble_fitimage_initramfs:prepend() {
    tdx_signed_hsm_vars_export
}
do_assemble_fitimage_initramfs[lockfiles] = "${TMPDIR}/locks/hsm.lock"

do_assemble_fitimage:prepend() {
    tdx_signed_hsm_vars_export
}
do_assemble_fitimage[lockfiles] = "${TMPDIR}/locks/hsm.lock"
