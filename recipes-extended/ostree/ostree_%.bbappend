FILESEXTRAPATHS:prepend := "${THISDIR}/files:"

SRC_URI:append:tdx-signed:x86-64 = " \
    file://0001-sysroot-add-boot-counting-suffix-to-new-BLS-entry.patch \
    file://0002-deploy-add-support-for-uki.patch \
    file://0003-ostree-add-the-support-for-reading-extra-kernel-comm.patch \
"
