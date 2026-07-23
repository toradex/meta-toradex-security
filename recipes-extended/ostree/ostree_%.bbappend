FILESEXTRAPATHS:prepend := "${THISDIR}/files:"

SRC_URI:append:intel-corei7-64:tdx-signed = " \
    file://0001-sysroot-add-boot-counting-suffix-to-new-BLS-entry.patch \
"
