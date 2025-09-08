SUMMARY = "Script to handle encryption on Toradex modules"

LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

SRC_URI = "\
    file://tdx-enc.sh \
    file://tdx-enc \
    file://tdx-enc-handler.service \
    file://99-tpm.rules \
"

RDEPENDS:${PN} = "\
    openssl-bin \
    cryptsetup \
    e2fsprogs-mke2fs \
    keyutils \
    util-linux \
"

RDEPENDS_TPM = "\
    tpm2-tools \
"

RDEPENDS:${PN}:append = "${@ '${RDEPENDS_TPM}' if d.getVar('TDX_ENC_KEY_BACKEND') == 'tpm' else ''}"

inherit update-rc.d systemd

INITSCRIPT_NAME = "tdx-enc"
INITSCRIPT_PARAMS = "start 30 1 2 3 4 5 . stop 80 0 6 ."

SYSTEMD_SERVICE:${PN} = "tdx-enc-handler.service"

do_install() {
    install -d ${D}${sbindir}
    install -m 0755 ${WORKDIR}/tdx-enc.sh ${D}${sbindir}/tdx-enc.sh

    sed -i 's|@@TDX_ENC_KEY_BACKEND@@|${TDX_ENC_KEY_BACKEND}|g' ${D}${sbindir}/tdx-enc.sh
    sed -i 's|@@TDX_ENC_KEY_LOCATION@@|${TDX_ENC_KEY_LOCATION}|g' ${D}${sbindir}/tdx-enc.sh
    sed -i 's|@@TDX_ENC_KEY_FILE@@|${TDX_ENC_KEY_FILE}|g' ${D}${sbindir}/tdx-enc.sh
    sed -i 's|@@TDX_ENC_STORAGE_LOCATION@@|${TDX_ENC_STORAGE_LOCATION}|g' ${D}${sbindir}/tdx-enc.sh
    sed -i 's|@@TDX_ENC_STORAGE_RESERVE@@|${TDX_ENC_STORAGE_RESERVE}|g' ${D}${sbindir}/tdx-enc.sh
    sed -i 's|@@TDX_ENC_STORAGE_MOUNTPOINT@@|${TDX_ENC_STORAGE_MOUNTPOINT}|g' ${D}${sbindir}/tdx-enc.sh
    sed -i 's|@@TDX_ENC_STORAGE_MKFS_ARGS@@|${TDX_ENC_STORAGE_MKFS_ARGS}|g' ${D}${sbindir}/tdx-enc.sh
    sed -i 's|@@TDX_ENC_STORAGE_MOUNT_ARGS@@|${TDX_ENC_STORAGE_MOUNT_ARGS}|g' ${D}${sbindir}/tdx-enc.sh
    sed -i 's|@@TDX_ENC_KEY_DIR@@|${TDX_ENC_KEY_DIR}|g' ${D}${sbindir}/tdx-enc.sh
    sed -i 's|@@TDX_ENC_PRESERVE_DATA@@|${TDX_ENC_PRESERVE_DATA}|g' ${D}${sbindir}/tdx-enc.sh
    sed -i 's|@@TDX_ENC_BACKUP_STORAGE_PCT@@|${TDX_ENC_BACKUP_STORAGE_PCT}|g' ${D}${sbindir}/tdx-enc.sh

    install -d ${D}${systemd_system_unitdir}
    install -m 0644 ${WORKDIR}/tdx-enc-handler.service ${D}${systemd_system_unitdir}

    # setup systemd service dependencies
    if [ ${TDX_ENC_KEY_BACKEND} = "tpm" ]; then
        dep_bef="Before=local-fs.target"
        dep_aft="After=systemd-remount-fs.service dev-tpm0.device"
        dep_req="Requires=dev-tpm0.device"
        dep_all="${dep_bef}\n${dep_aft}\n${dep_req}"
    elif [ ${TDX_ENC_KEY_BACKEND} = "tee" ]; then
        dep_aft="After=tee-supplicant@teepriv0.service"
        dep_req="Requires=tee-supplicant@teepriv0.service"
        dep_all="${dep_aft}\n${dep_req}"
    else
        dep_bef="Before=local-fs.target"
        dep_aft="After=systemd-remount-fs.service"
        dep_all="${dep_bef}\n${dep_aft}"
    fi
    sed -i "/^@@DEPENDENCIES@@/c${dep_all}" ${D}${systemd_system_unitdir}/tdx-enc-handler.service

    install -d ${D}${sysconfdir}/init.d
    install -m 755 ${WORKDIR}/tdx-enc ${D}${sysconfdir}/init.d/tdx-enc

    if [ ${TDX_ENC_KEY_BACKEND} = "tpm" ]; then
        mkdir -p ${D}${sysconfdir}/udev/rules.d/
        install -m 0644 ${WORKDIR}/99-tpm.rules ${D}${sysconfdir}/udev/rules.d/99-tpm.rules
    fi
}
