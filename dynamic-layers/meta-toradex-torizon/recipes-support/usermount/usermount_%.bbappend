do_install:append:tdx-encrypted() {
    install -d ${D}${sysconfdir}/usermount
    echo ${TDX_ENC_STORAGE_LOCATION} >> ${D}${sysconfdir}/usermount/ignorelist
}
