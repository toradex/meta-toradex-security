SRC_URI:append = "\
    file://u-boot-hab.cfg \
"

SRC_URI:append:use-nxp-bsp = "\
    file://0001-bootm-don-t-use-HAB-to-authenticate-FIT-image.patch \
"
