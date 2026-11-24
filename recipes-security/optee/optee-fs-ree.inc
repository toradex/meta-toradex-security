FILESEXTRAPATHS:prepend := "${THISDIR}/files:"

SRC_URI += "\
    file://tee-supplicant-storage.conf \
    file://tee-supplicant-storage.systemd \
    file://tee-supplicant-storage.init \
"

DEPENDS += "${@bb.utils.contains('DISTRO_FEATURES', 'sysvinit', 'update-rc.d-native', '', d)}"

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
    elif ${@bb.utils.contains('DISTRO_FEATURES', 'sysvinit', 'true', 'false', d)}; then
        local script="${D}${sysconfdir}/init.d/tee-supplicant-storage"

        install -D -m 0755 ${UNPACKDIR}/tee-supplicant-storage.init ${script}
        sed -i -e 's|@@TDX_OPTEE_FS_PARENT_PATH@@|${TDX_OPTEE_FS_PARENT_PATH}|g' ${script}

        # update-rc.d.bbclass handles only one script per package, and this one
        # must run before tee-supplicant (start 10)
        update-rc.d -r ${D} tee-supplicant-storage start 09 1 2 3 4 5 .
    fi
}
