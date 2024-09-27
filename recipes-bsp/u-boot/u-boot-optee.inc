FILESEXTRAPATHS:prepend := "${THISDIR}/files:"

SRC_URI:append:verdin-imx8mp = "\
    file://0001-imx8mp-binman-add-tee-node.patch \
    file://dram-banks.cfg \
"

IMX_BOOT_CONTAINER_FIRMWARE:append = "tee.bin"
