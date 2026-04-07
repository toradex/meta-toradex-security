inherit tdx-signed-hsm-env

do_uboot_assemble_fitimage:prepend() {
    tdx_signed_hsm_vars_export
}
do_uboot_assemble_fitimage[lockfiles] = "${TMPDIR}/locks/hsm.lock"
