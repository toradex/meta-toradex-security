FILESEXTRAPATHS:prepend := "${THISDIR}/files:"

inherit imx-hab

SRC_URI:append = "\
    file://mx8m_create_csf.sh \
    file://mx8m_template.csf \
    file://create_fuse_cmds.sh \
    file://mx8_sign.sh \
    file://mx8_template.csf \
"

sign_ahab_helper() {
    MKIMAGE_BUILD_LOG=${WORKDIR}/mkimage.log

    for target in ${IMXBOOT_TARGETS}; do
        IMXBOOT_IMAGE="${S}/${BOOT_CONFIG_MACHINE_EXTRA}-${target}"

        # re-generate flash.bin and save logs
        make SOC=${IMX_BOOT_SOC_TARGET} ${REV_OPTION} dtbs=${UBOOT_DTB_NAME_EXTRA} \
             ${target} > ${MKIMAGE_BUILD_LOG} 2>&1

        # sign flash.bin
        TDX_IMX_HAB_CST_BIN="${TDX_IMX_HAB_CST_BIN}" \
        TDX_IMX_HAB_CST_SRK="${TDX_IMX_HAB_CST_SRK}" \
        TDX_IMX_HAB_CST_SRK_CERT="${TDX_IMX_HAB_CST_SRK_CERT}" \
        TDX_IMX_HAB_CST_SGK_CERT="${TDX_IMX_HAB_CST_SGK_CERT}" \
        UNSIGNED_IMAGE="${IMXBOOT_IMAGE}" \
        LOG_MKIMAGE="${MKIMAGE_BUILD_LOG}" \
        ${WORKDIR}/mx8_sign.sh -t ${target}

        # update flash.bin to signed image
        cp -v ${IMXBOOT_IMAGE} ${IMXBOOT_IMAGE}-unsigned
        cp -v ${IMXBOOT_IMAGE}-signed ${IMXBOOT_IMAGE}

        # create fuse commands
        ${WORKDIR}/create_fuse_cmds.sh \
                  ${IMX_BOOT_SOC_TARGET} ${TDX_IMX_HAB_CST_SRK_FUSE} \
                  ${WORKDIR}/fuse-cmds.txt
    done
}

sign_habv4_helper() {
    MKIMAGE_BUILD_LOG=${WORKDIR}/mkimage.log
    MKIMAGE_HAB_LOG=${WORKDIR}/mkimage.hab

    for target in ${IMXBOOT_TARGETS}; do
        IMXBOOT_IMAGE="${S}/${BOOT_CONFIG_MACHINE_EXTRA}-${target}"

        # re-generate flash.bin and save logs
        make SOC=${IMX_BOOT_SOC_TARGET} ${REV_OPTION} dtbs=${UBOOT_DTB_NAME_EXTRA} \
             ${target} > ${MKIMAGE_BUILD_LOG} 2>&1

        # calculate FIT HAB offset
        if [ ${target} = "flash_evk_emmc_fastboot" ]; then
            PRINT_FIT_HAB_OFFSET="0x68000"
        else
            PRINT_FIT_HAB_OFFSET="0x60000"
        fi

        # generate HAB info
        make SOC=${IMX_BOOT_SOC_TARGET} ${REV_OPTION} dtbs=${UBOOT_DTB_NAME_EXTRA} \
             PRINT_FIT_HAB_OFFSET=${PRINT_FIT_HAB_OFFSET} \
             print_fit_hab > ${MKIMAGE_HAB_LOG} 2>&1

        # generate SPL and FIT CSF files
        TDX_IMX_HAB_CST_SRK="${TDX_IMX_HAB_CST_SRK}" \
        TDX_IMX_HAB_CST_SRK_CERT="${TDX_IMX_HAB_CST_SRK_CERT}" \
        TDX_IMX_HAB_CST_CSF_CERT="${TDX_IMX_HAB_CST_CSF_CERT}" \
        TDX_IMX_HAB_CST_IMG_CERT="${TDX_IMX_HAB_CST_IMG_CERT}" \
        TDX_IMX_HAB_CST_BIN="${TDX_IMX_HAB_CST_BIN}" \
        IMXBOOT="${IMXBOOT_IMAGE}" \
        LOG_MKIMAGE="${MKIMAGE_BUILD_LOG}" \
        LOG_PRINT_FIT_HAB="${MKIMAGE_HAB_LOG}" \
        ${WORKDIR}/mx8m_create_csf.sh -t ${target}

        # get SPL and FIT CSF offset
        TDX_IMX_OFFSET_SPL="$(grep "^ *csf_off" "${MKIMAGE_BUILD_LOG}" | awk '{print $NF}')"
        TDX_IMX_OFFSET_FIT="$(grep "^ *sld_csf_off" "${MKIMAGE_BUILD_LOG}" | awk '{print $NF}')"

        # save unsigned image
        cp -v ${IMXBOOT_IMAGE} ${IMXBOOT_IMAGE}-unsigned

        # insert SPL and FIT signatures
        dd if=${WORKDIR}/${target}-csf-spl.bin of=${IMXBOOT_IMAGE} \
           seek=$(printf "%d" ${TDX_IMX_OFFSET_SPL}) bs=1 conv=notrunc
        dd if=${WORKDIR}/${target}-csf-fit.bin of=${IMXBOOT_IMAGE} \
           seek=$(printf "%d" ${TDX_IMX_OFFSET_FIT}) bs=1 conv=notrunc

        # create fuse commands
        ${WORKDIR}/create_fuse_cmds.sh \
                  ${IMX_BOOT_SOC_TARGET} ${TDX_IMX_HAB_CST_SRK_FUSE} \
                  ${WORKDIR}/fuse-cmds.txt
    done
}

sign_common() {
    local helper_type="${1?Helper type expected (ahab or habv4)}"

    # The basic structure of this comes from meta-freescale/imx-boot_*.bb:
    for type in ${UBOOT_CONFIG}; do
        if [ "${@d.getVarFlags('UBOOT_DTB_NAME')}" = "None" ]; then
            UBOOT_DTB_NAME_FLAGS="${type}:${UBOOT_DTB_NAME}"
        else
            UBOOT_DTB_NAME_FLAGS="${@' '.join(flag + ':' + dtb for flag, dtb in (d.getVarFlags('UBOOT_DTB_NAME')).items()) if d.getVarFlags('UBOOT_DTB_NAME') is not None else '' }"
        fi

        for key_value in ${UBOOT_DTB_NAME_FLAGS}; do
            type_key="${key_value%%:*}"
            dtb_name="${key_value#*:}"

            if [ "$type_key" = "$type" ]; then
                bbnote "UBOOT_CONFIG = $type, UBOOT_DTB_NAME = $dtb_name"

                UBOOT_CONFIG_EXTRA="$type_key"
                if [ -e ${DEPLOY_DIR_IMAGE}/${BOOT_TOOLS}/${dtb_name}-${type} ] ; then
                    UBOOT_DTB_NAME_EXTRA="${dtb_name}-${type}"
                else
                    # backward compatibility
                    UBOOT_DTB_NAME_EXTRA="${dtb_name}"
                fi
                UBOOT_NAME_EXTRA="u-boot-${MACHINE}.bin-${UBOOT_CONFIG_EXTRA}"
                BOOT_CONFIG_MACHINE_EXTRA="${BOOT_NAME}-${MACHINE}-${UBOOT_CONFIG_EXTRA}.bin"

                # Copy again artifacts for current U-Boot config to BOOT_STAGING
                compile_${SOC_FAMILY}

                case "$helper_type" in
                    "habv4") sign_habv4_helper ;;
                    "ahab")  sign_ahab_helper  ;;
                    *)
                        bbfatal "bad parameter to sign_common()"
                        exit 1;;
                esac

                unset UBOOT_CONFIG_EXTRA
                unset UBOOT_DTB_NAME_EXTRA
                unset UBOOT_NAME_EXTRA
                unset BOOT_CONFIG_MACHINE_EXTRA
            fi

            unset type_key
            unset dtb_name
        done

        unset UBOOT_DTB_NAME_FLAGS
    done
    unset type
}

sign_ahab() {
    if [ ! -e "${TDX_IMX_HAB_CST_BIN}" ]; then
        bberror "Could not find CST binary at ${TDX_IMX_HAB_CST_BIN}."
        exit 1
    fi

    if [ ! -e "${TDX_IMX_HAB_CST_SRK_FUSE}" ]; then
        bberror "Could not find SRK fuse file '${TDX_IMX_HAB_CST_SRK_FUSE}'."
    fi

    sign_common ahab
}

sign_habv4() {
    if [ ! -e "${TDX_IMX_HAB_CST_BIN}" ]; then
        bberror "Could not find CST binary at ${TDX_IMX_HAB_CST_BIN}."
        exit 1
    fi

    if [ ! -e "${TDX_IMX_HAB_CST_SRK_FUSE}" ]; then
        bberror "Could not find SRK fuse file '${TDX_IMX_HAB_CST_SRK_FUSE}'."
    fi

    sign_common habv4
}

do_compile:append:mx8m-generic-bsp() {
    sign_habv4
}

do_compile:append:mx8qm-generic-bsp() {
    sign_ahab
}

do_compile:append:mx8x-generic-bsp() {
    sign_ahab
}

do_deploy:append() {
    if [ -e ${WORKDIR}/fuse-cmds.txt ]; then
        install -d ${DEPLOYDIR}
        install -m 0644 ${WORKDIR}/fuse-cmds.txt ${DEPLOYDIR}
    fi
}
