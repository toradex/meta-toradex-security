FILESEXTRAPATHS:prepend := "${THISDIR}/files:"

SRC_URI:append:verdin-imx8mm = "\
    file://u-boot-optee.cfg \
"

SRC_URI:append:verdin-imx8mp = "\
    file://u-boot-optee.cfg \
    file://dram-banks.cfg \
"

SRC_URI:append:toradex-smarc-imx8mp = "\
    file://u-boot-optee.cfg \
    file://dram-banks.cfg \
"

SRC_URI:append:mx7-generic-bsp = "\
    file://enable-sec-mode.cfg \
"

IMX_BOOT_CONTAINER_FIRMWARE:append = "tee.bin"
