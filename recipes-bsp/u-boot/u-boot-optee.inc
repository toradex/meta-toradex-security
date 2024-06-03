FILESEXTRAPATHS:prepend := "${THISDIR}/files:"

SRC_URI:append:verdin-imx8mp = "\
    file://dram-banks.cfg \
"
