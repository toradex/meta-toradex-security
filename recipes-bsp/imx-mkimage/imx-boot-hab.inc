FILESEXTRAPATHS:prepend := "${THISDIR}/files:"

inherit imx-hab

SRC_URI:append = "\
    file://mx8m_create_csf.sh \
    file://mx8m_template.csf \
"

do_compile:append:mx8m-generic-bsp() {
    MKIMAGE_BUILD_LOG=${WORKDIR}/mkimage.log
    MKIMAGE_HAB_LOG=${WORKDIR}/mkimage.hab

    if [ ! -e "${TDX_IMX_HAB_CST_BIN}" ]; then
        bberror "Could not find CST binary at ${TDX_IMX_HAB_CST_BIN}."
        exit 1
    fi

    for f in ${TDX_IMX_HAB_CST_SRK} ${TDX_IMX_HAB_CST_CSF_CERT} ${TDX_IMX_HAB_CST_IMG_CERT}
    do
        if [ ! -e "${f}" ]; then
            bberror "Could not find cert file at ${f}."
            exit 1
        fi
    done

    for target in ${IMXBOOT_TARGETS}; do
        IMXBOOT_IMAGE="${S}/${BOOT_CONFIG_MACHINE}-${target}"

        # re-generate flash.bin and save logs
        make SOC=${IMX_BOOT_SOC_TARGET} ${REV_OPTION} dtbs=${UBOOT_DTB_NAME} \
                 ${target} > ${MKIMAGE_BUILD_LOG} 2>&1

        # calculate FIT HAB offset
        if [ ${target} = "flash_evk_emmc_fastboot" ]; then
            PRINT_FIT_HAB_OFFSET="0x68000"
        else
            PRINT_FIT_HAB_OFFSET="0x60000"
        fi

        # generate HAB info
        make SOC=${IMX_BOOT_SOC_TARGET} PRINT_FIT_HAB_OFFSET=${PRINT_FIT_HAB_OFFSET} \
                 dtbs=${UBOOT_DTB_NAME} ${REV_OPTION} print_fit_hab > ${MKIMAGE_HAB_LOG} 2>&1

        # generate SPL and FIT CFS files
        TDX_IMX_HAB_CST_SRK="${TDX_IMX_HAB_CST_SRK}" \
        TDX_IMX_HAB_CST_CSF_CERT="${TDX_IMX_HAB_CST_CSF_CERT}" \
        TDX_IMX_HAB_CST_IMG_CERT="${TDX_IMX_HAB_CST_IMG_CERT}" \
        TDX_IMX_HAB_CST_BIN="${TDX_IMX_HAB_CST_BIN}" \
        IMXBOOT="${IMXBOOT_IMAGE}" \
        LOG_MKIMAGE="${MKIMAGE_BUILD_LOG}" \
        LOG_PRINT_FIT_HAB="${MKIMAGE_HAB_LOG}" \
        ${WORKDIR}/mx8m_create_csf.sh -t ${target}

        # get SPL and FIT CSF offset
        TDX_IMX_OFFSET_SPL="$(cat ${MKIMAGE_BUILD_LOG} | grep " csf_off" | awk '{print $NF}')"
        TDX_IMX_OFFSET_FIT="$(cat ${MKIMAGE_BUILD_LOG} | grep " sld_csf_off" | awk '{print $NF}')"

        # save unsigned image
        cp ${IMXBOOT_IMAGE} ${IMXBOOT_IMAGE}-unsigned

        # insert SPL and FIT signatures
        dd if=${WORKDIR}/${target}-csf-spl.bin of=${IMXBOOT_IMAGE} \
           seek=$(printf "%d" ${TDX_IMX_OFFSET_SPL}) bs=1 conv=notrunc
        dd if=${WORKDIR}/${target}-csf-fit.bin of=${IMXBOOT_IMAGE} \
           seek=$(printf "%d" ${TDX_IMX_OFFSET_FIT}) bs=1 conv=notrunc
    done
}
