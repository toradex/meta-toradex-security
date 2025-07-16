FILESEXTRAPATHS:prepend := "${THISDIR}/files:"

SRC_URI:append:verdin-imx8mp = "\
    file://0001-imx8mp-binman-add-tee-node.patch \
    file://dram-banks.cfg \
"

SRC_URI:append:verdin-imx8mm = "\
    file://0001-imx8mm-binman-add-tee-node.patch \
"

SRC_URI:append:mx6-generic-bsp = "\
    file://imx-v6-v7-optee.dtsi \
"

SRC_URI:append:mx6-generic-bsp = "\
    file://optee-imx6.cfg \
"

EXTRA_OEMAKE:append:mx6-generic-bsp = " TEE=tee.bin"

copy_optee_binary_to_uboot() {
    if [ -n "${UBOOT_CONFIG}" ]; then
        for config in ${UBOOT_MACHINE}; do
            i=$(expr $i + 1);
            for type in ${UBOOT_CONFIG}; do
                j=$(expr $j + 1);
                if [ $j -eq $i ]; then
                    bbnote "Copying tee.bin from ${DEPLOY_DIR_IMAGE} to ${B}/${config}/".
                    cp ${DEPLOY_DIR_IMAGE}/tee.bin ${B}/${config}/
                fi
            done
            unset j
        done
        unset i
    else
        bbnote "Copying tee.bin from ${DEPLOY_DIR_IMAGE} to ${B}/".
        cp ${DEPLOY_DIR_IMAGE}/tee.bin ${B}
    fi
}
do_prepare_uboot_for_optee() {
    bbnote "Nothing to be done for machine ${MACHINE}"
}
do_prepare_uboot_for_optee:mx6-generic-bsp() {
    copy_optee_binary_to_uboot
}
do_prepare_uboot_for_optee[depends] += "optee-os:do_deploy"
addtask do_prepare_uboot_for_optee before do_compile after do_configure

do_configure:append:mx6-generic-bsp() {
    # add binman node to U-Boot device tree
    sed -i 's|@@TDX_OPTEE_ADDR_LOAD@@|${TDX_OPTEE_ADDR_LOAD}|g' ${WORKDIR}/imx-v6-v7-optee.dtsi
    cp ${WORKDIR}/imx-v6-v7-optee.dtsi ${S}/arch/arm/dts/

    # make sure binman is selected for iMX6
    sed -i '/config MX6$/a\\tselect BINMAN' ${S}/arch/arm/mach-imx/mx6/Kconfig
}

IMX_BOOT_CONTAINER_FIRMWARE:append = "tee.bin"
