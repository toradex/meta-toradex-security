SUMMARY = "UEFI Secure Boot DB and DBX append update artifacts"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

COMPATIBLE_MACHINE = "intel-corei7-64"

require classes/include/signed/tdx-signed-uefi-secboot.inc

inherit deploy

do_deploy[depends] += "${UEFI_SB_NATIVE_TASK_DEPS}"
do_deploy[file-checksums] += "${UEFI_SB_KEY_FILE_CHECKSUMS}"
do_deploy[file-checksums] += "${UEFI_SB_UPDATE_INPUT_FILE_CHECKSUMS}"

do_deploy() {
    uefi_sb_make_update_auth
}

addtask deploy after do_compile before do_build
