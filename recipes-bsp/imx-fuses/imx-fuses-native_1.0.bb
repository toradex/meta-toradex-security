SUMMARY = "Script to create fusing-related configuration files for iMX SoCs"
DESCRIPTION = "Packages a script and template files for producing \
fusing-related configuration files for iMX SoCs"

LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

DEPENDS = "util-linux-native"

SRC_URI = "\
    file://create_fuse_cmds.sh \
    file://imx6-template.fuse \
    file://imx7-template.fuse \
    file://imx8m-template.fuse \
    file://imx8qm-template.fuse \
    file://imx8qx-template.fuse \
    file://imx95-template.fuse \
"

inherit native

do_install() {
    install -d ${D}${bindir}
    install -m 0755 ${WORKDIR}/create_fuse_cmds.sh ${D}${bindir}/create_fuse_cmds.sh

    install -d ${D}${datadir}/${BPN}
    install -m 0644 ${WORKDIR}/*-template.fuse ${D}${datadir}/${BPN}
}
