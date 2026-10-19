FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"

SRC_URI:append:tdx-encrypted = "\
    file://tdx-enc-handler-requirements.cfg \
"
