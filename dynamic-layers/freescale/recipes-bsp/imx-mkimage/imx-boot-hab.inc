FILESEXTRAPATHS:prepend := "${THISDIR}/files:"

inherit imx-hab

# Need this for the create_fuse_cmds.sh script
DEPENDS += "imx-fuses-native"

# build CST binary from sources
DEPENDS += "${@ 'imx-code-signing-tool-native' if d.getVar('TDX_IMX_HAB_CST_BUILD_FROM_SOURCE') == '1' else '' }"

SRC_URI:append = "\
    file://mx8_sign.sh \
    file://mx8_template.csf \
    file://mx9_template.csf \
"

TDX_IMX_HAB_CST_TEMPLATE:mx8-generic-bsp = "mx8_template.csf"
TDX_IMX_HAB_CST_TEMPLATE:mx9-generic-bsp = "mx9_template.csf"

sign_ahab_image() {
    local orig_image=$1
    local dest_image=$2
    local make_target=$3

    local logfile=${WORKDIR}/mkimage-${make_target}.log

    # re-generate image and save logs
    make SOC=${IMX_BOOT_SOC_TARGET} ${REV_OPTION} dtbs=${UBOOT_DTB_NAME_EXTRA} \
         ${MKIMAGE_EXTRA_ARGS} ${make_target} > ${logfile} 2>&1

    # sign image
    TDX_IMX_HAB_CST_BIN="${TDX_IMX_HAB_CST_BIN}" \
    TDX_IMX_HAB_CST_ARGS="${TDX_IMX_HAB_CST_ARGS}" \
    TDX_IMX_HAB_CST_SRK="${TDX_IMX_HAB_CST_SRK}" \
    TDX_IMX_HAB_CST_SRK_CERT="${TDX_IMX_HAB_CST_SRK_CERT}" \
    TDX_IMX_HAB_CST_SGK_CERT="${TDX_IMX_HAB_CST_SGK_CERT}" \
    TDX_IMX_HAB_CST_SGK_SUPP="${TDX_IMX_HAB_CST_SGK_SUPP}" \
    TDX_IMX_HAB_CST_TEMPLATE="${TDX_IMX_HAB_CST_TEMPLATE}" \
    TDX_IMX_HAB_CST_HSM="${TDX_SIGNED_HSM}" \
    TDX_IMX_HAB_CST_SRK_CA="${TDX_IMX_HAB_CST_SRK_CA}" \
    TDX_IMX_HAB_CST_SRK_INDEX="${TDX_IMX_HAB_CST_SRK_INDEX}" \
    UNSIGNED_IMAGE="${orig_image}" \
    LOG_MKIMAGE="${logfile}" \
    ${WORKDIR}/mx8_sign.sh -t ${make_target}

    # copy signed image
    cp -v ${orig_image}-signed ${dest_image}
}

sign_ahab_helper() {
    # sign boot container
    for target in ${IMXBOOT_TARGETS}; do
        local imgfile=""

        # On iMX95, the u-boot-atf-container.img is generated separately
        # and appended to flash.bin later. Therefore, it must be signed
        # before creating and signing the final boot container image.
        if [ "${IMX_BOOT_SOC_TARGET}" = "iMX95" ]; then
            local imgdir="${S}/iMX95"
            local imgname="u-boot-atf-container.img"
            imgfile="${imgdir}/${imgname}"

            rm -rf "${imgfile}"
            sign_ahab_image "${imgfile}" "${imgfile}" "${imgname}"
        fi

        imgfile="${S}/${IMX_BOOT_SOC_TARGET}/flash.bin"
        sign_ahab_image "${imgfile}" "${S}/${BOOT_CONFIG_MACHINE_EXTRA}-${target}" "${target}"
    done

    # create fuse commands
    ${STAGING_BINDIR_NATIVE}/create_fuse_cmds.sh \
        "${IMX_BOOT_SOC_TARGET}" \
        "${TDX_IMX_HAB_CST_SRK_FUSE}" \
        "${WORKDIR}/fuse-cmds.txt" \
        "${RECIPE_SYSROOT_NATIVE}${datadir}/imx-fuses"
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

    # show warning if CA flag is enabled but not supported
    # when an HSM is used, the TDX_IMX_HAB_CST_SRK_CERT is actually a PKCS#11 URI and
    # cannot be used to check if the CA flag is enabled, so in this case we use the
    # TDX_IMX_HAB_CST_SRK_CA variable to check if CA is enabled
    local ca_flag_warn=0
    if [ "${TDX_SIGNED_HSM}" = "1" ]; then
        if [ "${TDX_IMX_HAB_CST_SRK_CA}" = "1" ] && [ "${TDX_IMX_HAB_CST_SGK_SUPP}" = "0" ]; then
            ca_flag_warn=1
        fi
    else
        local srk_cert=${TDX_IMX_HAB_CST_SRK_CERT}
        if [ "${srk_cert##*_ca_}" = "crt.pem" ] && [ "${TDX_IMX_HAB_CST_SGK_SUPP}" = "0" ]; then
            ca_flag_warn=1
        fi
    fi
    if [ "${ca_flag_warn}" = "1" ]; then
        bbwarn "AHAB (Secure Boot) signing keys seem to have the CA flag enabled, but" \
               "the build target [${MACHINE}] does not support the use of subordinate" \
               "signing keys. Bootloader will be signed using SRK directly."
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
                BOOT_CONFIG_MACHINE_EXTRA="imx-boot-${MACHINE}-${UBOOT_CONFIG_EXTRA}.bin"

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

do_compile:append:mx9-generic-bsp() {
    sign_common ahab
}

do_deploy:append() {
    if [ -e ${WORKDIR}/fuse-cmds.txt ]; then
        install -d ${DEPLOYDIR}
        install -m 0644 ${WORKDIR}/fuse-cmds.txt ${DEPLOYDIR}
        install -m 0644 ${WORKDIR}/imx-config.fuse ${DEPLOYDIR}
    fi
}

require ${@oe.utils.conditional('TDX_SIGNED_HSM', '1', 'imx-boot-hab-hsm.inc', '', d)}
