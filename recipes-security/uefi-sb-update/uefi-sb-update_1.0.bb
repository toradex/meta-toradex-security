SUMMARY = "UEFI Secure Boot DB and DBX subsystem update handler"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

COMPATIBLE_MACHINE = "intel-corei7-64"

SRC_URI = " \
    file://apply-uefi-auth.sh \
    file://apply-uefi-auth-handler.sh \
"

RDEPENDS:${PN} = "bash coreutils e2fsprogs efitools"

do_install() {
    install -d ${D}${libexecdir}/uefi-sb-update
    install -m 0755 ${WORKDIR}/apply-uefi-auth.sh \
        ${D}${libexecdir}/uefi-sb-update/apply-uefi-auth.sh
    install -m 0755 ${WORKDIR}/apply-uefi-auth-handler.sh \
        ${D}${libexecdir}/uefi-sb-update/apply-uefi-auth-handler.sh
}

FILES:${PN} = "${libexecdir}/uefi-sb-update"
