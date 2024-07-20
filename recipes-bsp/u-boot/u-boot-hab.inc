inherit imx-hab

# Need this for hexdump utility which is used in create_fuse_cmds.sh
DEPENDS += "util-linux-native"

SRC_URI:append = "\
    file://u-boot-hab.cfg \
    file://create_csf.sh \
    file://create_fuse_cmds.sh \
    file://imx6ull_template.csf \
    file://imx7_template.csf \
    file://imx6_template.csf \
"

TDX_UBOOT_HAB_PATCHES_DOWNSTREAM = "\
    file://0001-bootm-don-t-use-HAB-to-authenticate-FIT-image.patch \
"

SRC_URI:append:apalis-imx8 = "${TDX_UBOOT_HAB_PATCHES_DOWNSTREAM}"
SRC_URI:append:colibri-imx8x = "${TDX_UBOOT_HAB_PATCHES_DOWNSTREAM}"

UBOOT_BINARY_HAB = "${UBOOT_BINARY}"
UBOOT_BINARY_HAB:mx6dl-generic-bsp = "u-boot-ivt.img"
UBOOT_BINARY_HAB:mx6q-generic-bsp = "u-boot-ivt.img"

# Sign U-Boot binary and/or SPL.
# This is supposed to run in the U-Boot build directory.
sign_habv4() {
    local soc="$1"
    local type="$2"
    local binary="$3"

    if [ ! -e "${TDX_IMX_HAB_CST_BIN}" ]; then
        bbfatal "Could not find CST binary at ${TDX_IMX_HAB_CST_BIN}."
    fi

    for f in "${TDX_IMX_HAB_CST_SRK}" "${TDX_IMX_HAB_CST_CSF_CERT}" \
             "${TDX_IMX_HAB_CST_IMG_CERT}" "${TDX_IMX_HAB_CST_SRK_FUSE}"; do
        if [ ! -e "${f}" ]; then
            bbfatal "Could not find file '${f}' (required for HAB)."
        fi
    done

    bbdebug 1 "Generating CSF for U-Boot"
    # Generate CSF file:
    TDX_IMX_HAB_CST_SRK="${TDX_IMX_HAB_CST_SRK}" \
    TDX_IMX_HAB_CST_CSF_CERT="${TDX_IMX_HAB_CST_CSF_CERT}" \
    TDX_IMX_HAB_CST_IMG_CERT="${TDX_IMX_HAB_CST_IMG_CERT}" \
    TDX_IMX_HAB_CST_BIN="${TDX_IMX_HAB_CST_BIN}" \
    IMXBOOT="${UBOOT_BINARY_HAB}" \
    HAB_LOG="${UBOOT_BINARY_HAB}.log" \
    ${WORKDIR}/create_csf.sh -m "${soc}" -c "csf_uboot"

    # Save unsigned image and generate signed version by concatenating with the CSF.
    bbdebug 1 "Concatenating CSF binary to U-Boot"
    if [ -n "${type}" ]; then
        cp  "${UBOOT_BINARY_HAB}" "${UBOOT_BINARYNAME}-${type}.${UBOOT_SUFFIX}-unsigned"
        cat "${UBOOT_BINARY_HAB}" "csf_uboot.bin" > "${UBOOT_BINARYNAME}-${type}.${UBOOT_SUFFIX}"
    else
        mv  "${UBOOT_BINARY_HAB}" "${UBOOT_BINARY}-unsigned"
        cat "${UBOOT_BINARY}-unsigned" "csf_uboot.bin" > "${UBOOT_BINARY}"
    fi

    # Repeat the process for SPL if SPL was also built.
    if [ -n "${SPL_BINARY}" ]; then
        bbdebug 1 "Generating CSF for the U-Boot SPL"
        TDX_IMX_HAB_CST_SRK="${TDX_IMX_HAB_CST_SRK}" \
        TDX_IMX_HAB_CST_CSF_CERT="${TDX_IMX_HAB_CST_CSF_CERT}" \
        TDX_IMX_HAB_CST_IMG_CERT="${TDX_IMX_HAB_CST_IMG_CERT}" \
        TDX_IMX_HAB_CST_BIN="${TDX_IMX_HAB_CST_BIN}" \
        IMXBOOT="${SPL_BINARY}" \
        HAB_LOG="${SPL_BINARY}.log" \
        ${WORKDIR}/create_csf.sh -m "${soc}" -c "csf_SPL"

        bbdebug 1 "Concatenating CSF binary to the U-Boot SPL"
        # Save unsigned SPL and replace original with signed one:
        mv "${SPL_BINARY}" "${SPL_BINARY}-unsigned"
        cat "${SPL_BINARY}-unsigned" "csf_SPL.bin" > "${SPL_BINARY}"

        # TODO: Regenerate u-boot-with-spl.imx
    fi

    # Create fuse commands
    ${WORKDIR}/create_fuse_cmds.sh "${soc}" \
                                   "${TDX_IMX_HAB_CST_SRK_FUSE}" \
                                   "${WORKDIR}/fuse-cmds.txt"
}

uboot_assemble_fitimage_helper:append:mx6dl-generic-bsp() {
    type="$1"
    binary="$2"
    sign_habv4 "IMX6" "${type}" "${binary}"
}

uboot_assemble_fitimage_helper:append:mx6q-generic-bsp() {
    type="$1"
    binary="$2"
    sign_habv4 "IMX6" "${type}" "${binary}"
}

uboot_assemble_fitimage_helper:append:mx6ull-generic-bsp() {
    type="$1"
    binary="$2"
    sign_habv4 "IMX6ULL" "${type}" "${binary}"
}

uboot_assemble_fitimage_helper:append:mx7-generic-bsp() {
    type="$1"
    binary="$2"
    sign_habv4 "IMX7" "${type}" "${binary}"
}

do_deploy:append() {
    if [ -e ${WORKDIR}/fuse-cmds.txt ]; then
        install -d ${DEPLOYDIR}
        install -m 0644 ${WORKDIR}/fuse-cmds.txt ${DEPLOYDIR}
    fi
}
