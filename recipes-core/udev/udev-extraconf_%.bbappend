do_install:append:tdx-encrypted() {
    echo ${TDX_ENC_STORAGE_LOCATION} >> ${D}${sysconfdir}/udev/mount.ignorelist
}
