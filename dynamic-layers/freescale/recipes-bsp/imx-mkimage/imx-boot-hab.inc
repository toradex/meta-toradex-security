FILESEXTRAPATHS:prepend := "${THISDIR}/files:"

inherit imx-hab

SRC_URI:append = "\
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
    bbfatal "Generating the bootloader container via imx-boot is no longer supported;" \
            "please review your setting of UBOOT_PROVIDES_BOOT_CONTAINER."
}

sign_common() {
    local helper_type="${1?Helper type expected (ahab or habv4)}"

    if [ ! -e "${TDX_IMX_HAB_CST_BIN}" ]; then
        bberror "Could not find CST binary at ${TDX_IMX_HAB_CST_BIN}."
        exit 1
    fi

    if [ ! -e "${TDX_IMX_HAB_CST_SRK_FUSE}" ]; then
        bberror "Could not find SRK fuse file '${TDX_IMX_HAB_CST_SRK_FUSE}'."
    fi

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

do_compile:append:mx8m-generic-bsp() {
    sign_common habv4
}

do_compile:append:mx8qm-generic-bsp() {
    sign_common ahab
}

do_compile:append:mx8x-generic-bsp() {
    sign_common ahab
}

do_deploy:append() {
    if [ -e ${WORKDIR}/fuse-cmds.txt ]; then
        install -d ${DEPLOYDIR}
        install -m 0644 ${WORKDIR}/fuse-cmds.txt ${DEPLOYDIR}
    fi
}
