FILESEXTRAPATHS:prepend:intel-corei7-64:tdx-signed := "${THISDIR}/intel-corei7-64:"

RDEPENDS:${PN}:append:intel-corei7-64:tdx-signed = " systemd ostree"

do_install:append:intel-corei7-64:tdx-signed() {
    # Mask systemd-bless-boot.service so it does not auto-confirm boots before
    # greenboot runs. The greenboot green.d/red.d scripts will invoke the
    # systemd-bless-boot binary directly and are the sole boot-assessment authority.
    install -d ${D}${sysconfdir}/systemd/system
    ln -s /dev/null ${D}${sysconfdir}/systemd/system/systemd-bless-boot.service
}

FILES:${PN}:append:intel-corei7-64:tdx-signed = "\
    ${sysconfdir}/systemd/system/systemd-bless-boot.service \
"
