FILESEXTRAPATHS:prepend := "${THISDIR}/files:"

SRC_URI:append:verdin-imx8mp = "\
    file://0001-imx8mp-binman-add-tee-node.patch \
    file://dram-banks.cfg \
"

SRC_URI:append:verdin-imx8mm = "\
    file://0001-imx8mm-binman-add-tee-node.patch \
"

IMX_BOOT_CONTAINER_FIRMWARE:append = "tee.bin"
