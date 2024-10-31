FILESEXTRAPATHS:prepend := "${THISDIR}/files:"

SRC_URI += "\
    file://tee-supplicant.rules \
"

# The optee-client upstream recipe installs a tee-supplicant template unit
# that requires an udev rule to start the tee supplicant daemon automatically
do_install:append() {
    if [ -e "${D}${systemd_system_unitdir}/tee-supplicant@.service" ]; then
        install -d ${D}${nonarch_base_libdir}/udev/rules.d/
        install -m 755 ${WORKDIR}/tee-supplicant.rules ${D}${nonarch_base_libdir}/udev/rules.d/
    fi
}
