FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"

SRC_URI:append = "\
    ${@oe.utils.conditional('TDX_ENC_ENABLE', '1', 'file://tdx-enc-handler-requirements.cfg', '', d)} \
"
