FILESEXTRAPATHS:prepend := "${THISDIR}/files:"

SRC_URI += "\
    file://tee-supplicant-storage.conf \
    file://tee-supplicant-storage.systemd \
"

FILES:${PN} += "${systemd_system_unitdir}/tee-supplicant@.service.d"

# tee-supplicant doesn't run as root, so it needs to own the secure storage directory
do_install:append() {
    if ${@bb.utils.contains('DISTRO_FEATURES', 'systemd', 'true', 'false', d)}; then
        local conf="${datadir}/${BPN}/tee-supplicant-storage.conf"
        local dropin="${D}${systemd_system_unitdir}/tee-supplicant@.service.d/storage.conf"

        install -D -m 0644 ${UNPACKDIR}/tee-supplicant-storage.conf ${D}${conf}
        sed -i -e 's|@@TDX_OPTEE_FS_PARENT_PATH@@|${TDX_OPTEE_FS_PARENT_PATH}|g' ${D}${conf}

        install -D -m 0644 ${UNPACKDIR}/tee-supplicant-storage.systemd ${dropin}
        sed -i -e 's|@@TDX_OPTEE_FS_PARENT_PATH@@|${TDX_OPTEE_FS_PARENT_PATH}|g' \
               -e 's|@@BINDIR@@|${bindir}|g' \
               -e "s|@@CONF@@|${conf}|g" \
               ${dropin}
    fi
}
