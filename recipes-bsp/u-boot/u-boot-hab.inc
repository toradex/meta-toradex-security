inherit imx-hab

# Need this for hexdump utility which is used in create_fuse_cmds.sh
DEPENDS += "util-linux-native"

SRC_URI:append = "\
    file://u-boot-hab.cfg \
    file://create_csf.sh \
    file://create_fuse_cmds.sh \
    file://imx6ull_template.csf \
"

SRC_URI:append:use-nxp-bsp = "\
    file://0001-bootm-don-t-use-HAB-to-authenticate-FIT-image.patch \
"

sign_habv4() {
    if [ ! -e "${TDX_IMX_HAB_CST_BIN}" ]; then
        bberror "Could not find CST binary at ${TDX_IMX_HAB_CST_BIN}."
        exit 1
    fi

    for f in "${TDX_IMX_HAB_CST_SRK}" "${TDX_IMX_HAB_CST_CSF_CERT}" \
             "${TDX_IMX_HAB_CST_IMG_CERT}" "${TDX_IMX_HAB_CST_SRK_FUSE}"
    do
        if [ ! -e "${f}" ]; then
            bberror "Could not find cert file at ${f}."
            exit 1
        fi
    done

    local soc="$1"

    if [ -n "${UBOOT_CONFIG}" ]; then
        i=0
        j=0
        for config in ${UBOOT_MACHINE}; do
            i=$(expr $i + 1);
            for type in ${UBOOT_CONFIG}; do
                j=$(expr $j + 1);
                if [ $j -eq $i ]; then
		    # TODO: finish this when working on machines with an SPL
		    bbnote "Machines with UBOOT_CONFIG set, are not implemented yet."
                fi
            done
        done
    else
        # Generate CSF file
        TDX_IMX_HAB_CST_SRK="${TDX_IMX_HAB_CST_SRK}" \
        TDX_IMX_HAB_CST_CSF_CERT="${TDX_IMX_HAB_CST_CSF_CERT}" \
        TDX_IMX_HAB_CST_IMG_CERT="${TDX_IMX_HAB_CST_IMG_CERT}" \
        TDX_IMX_HAB_CST_BIN="${TDX_IMX_HAB_CST_BIN}" \
	IMXBOOT="${B}/${UBOOT_BINARY}" \
        HAB_LOG="${B}/${UBOOT_MAKE_TARGET}.log" \
        ${WORKDIR}/create_csf.sh -m "${soc}"

	# Save unsigned image
	mv ${UBOOT_BINARY} ${UBOOT_BINARY}-unsigned

	# Append CSF to image
	cat ${UBOOT_BINARY}-unsigned ${WORKDIR}/csf-uboot.bin > ${UBOOT_BINARY}

	# Create fuse commands
	${WORKDIR}/create_fuse_cmds.sh ${soc} ${TDX_IMX_HAB_CST_SRK_FUSE} ${WORKDIR}/fuse-cmds.txt
    fi
}

do_compile:append:mx6ull-generic-bsp() {
    # Only sign here if FIT image signing is disabled.
    # Otherwise we sign later instead (see u-boot-fit-signature.inc).
    if [ "${UBOOT_SIGN_ENABLE}" != "1" ]; then
        sign_habv4 "IMX6ULL"
    fi
}

do_deploy:append() {
    if [ -e ${WORKDIR}/fuse-cmds.txt ]; then
        install -d ${DEPLOYDIR}
        install -m 0644 ${WORKDIR}/fuse-cmds.txt ${DEPLOYDIR}
    fi
}
