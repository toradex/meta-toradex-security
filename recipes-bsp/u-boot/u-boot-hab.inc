inherit imx-hab

# Need this for the create_fuse_cmds.sh script
DEPENDS += "imx-fuses-native"

# Need this for the xxd and fdtget utilities used by imx8m_sign.sh
DEPENDS += "xxd-native dtc-native"

# Need this to create deterministic CSF binaries when running the NXP Code Signing Tool
DEPENDS += "libfaketime-native"

# build CST binary from sources
DEPENDS += "${@ 'imx-code-signing-tool-native' if d.getVar('TDX_IMX_HAB_CST_BUILD_FROM_SOURCE') == '1' else '' }"

SRC_URI:append = "\
    file://u-boot-hab.cfg \
"

TDX_IMX6_IMX7_CSF_ARTIFACTS = "\
    file://imx6_imx7_create_csf.sh \
    file://imx6_template.csf \
    file://imx6ull_template.csf \
    file://imx7_template.csf \
"
SRC_URI:append:mx6-generic-bsp = "${TDX_IMX6_IMX7_CSF_ARTIFACTS}"
SRC_URI:append:mx7-generic-bsp = "${TDX_IMX6_IMX7_CSF_ARTIFACTS}"

TDX_IMX8M_CSF_ARTIFACTS = "\
    file://imx8m_sign.sh \
    file://imx8m_template.csf \
"
SRC_URI:append:mx8m-generic-bsp = "${TDX_IMX8M_CSF_ARTIFACTS}"

TDX_UBOOT_HAB_PATCHES_DOWNSTREAM = "\
    file://0001-bootm-don-t-use-HAB-to-authenticate-FIT-image.patch \
"
SRC_URI:append:apalis-imx8 = "${TDX_UBOOT_HAB_PATCHES_DOWNSTREAM}"
SRC_URI:append:colibri-imx8x = "${TDX_UBOOT_HAB_PATCHES_DOWNSTREAM}"

UBOOT_BINARY_HAB = "${UBOOT_BINARY}"
UBOOT_BINARY_HAB:mx6dl-generic-bsp = "u-boot-ivt.img"
UBOOT_BINARY_HAB:mx6q-generic-bsp = "u-boot-ivt.img"

# Call create_fuse_cmds.sh with all required env vars and parameters
create_fuse_cmds_helper() {
    local soc="$1"

    TDX_IMX_HAB_CST_SRK_FUSE="${TDX_IMX_HAB_CST_SRK_FUSE}" \
    ${STAGING_BINDIR_NATIVE}/create_fuse_cmds.sh \
        "$soc" \
        "${WORKDIR}/fuse-cmds.txt" \
        "${RECIPE_SYSROOT_NATIVE}${datadir}/imx-fuses"
}

# Sign U-Boot binary and/or SPL (this works both with i.MX6 and i.MX7).
# This is supposed to run in the U-Boot build directory.
imx6_imx7_sign_habv4() {
    local soc="$1"
    local type="$2"
    local binary="$3"

    if [ ! -e "${TDX_IMX_HAB_CST_BIN}" ]; then
        bbfatal "Could not find CST binary at ${TDX_IMX_HAB_CST_BIN}."
    fi

    bbdebug 1 "Generating CSF for U-Boot"
    # Generate CSF file:
    LIBFAKETIME_PATH="${STAGING_LIBDIR_NATIVE}/faketime/libfaketime.so.1" \
    TDX_IMX_HAB_CST_SRK="${TDX_IMX_HAB_CST_SRK}" \
    TDX_IMX_HAB_CST_SRK_CERT="${TDX_IMX_HAB_CST_SRK_CERT}" \
    TDX_IMX_HAB_CST_CSF_CERT="${TDX_IMX_HAB_CST_CSF_CERT}" \
    TDX_IMX_HAB_CST_IMG_CERT="${TDX_IMX_HAB_CST_IMG_CERT}" \
    TDX_IMX_HAB_CST_BIN="${TDX_IMX_HAB_CST_BIN}" \
    TDX_IMX_HAB_CST_ARGS="${TDX_IMX_HAB_CST_ARGS}" \
    IMXBOOT="${UBOOT_BINARY_HAB}" \
    HAB_LOG="${UBOOT_BINARY_HAB}.log" \
    ${WORKDIR}/imx6_imx7_create_csf.sh -m "${soc}" -c "csf_uboot"

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
        LIBFAKETIME_PATH="${STAGING_LIBDIR_NATIVE}/faketime/libfaketime.so.1" \
        TDX_IMX_HAB_CST_SRK="${TDX_IMX_HAB_CST_SRK}" \
        TDX_IMX_HAB_CST_SRK_CERT="${TDX_IMX_HAB_CST_SRK_CERT}" \
        TDX_IMX_HAB_CST_CSF_CERT="${TDX_IMX_HAB_CST_CSF_CERT}" \
        TDX_IMX_HAB_CST_IMG_CERT="${TDX_IMX_HAB_CST_IMG_CERT}" \
        TDX_IMX_HAB_CST_BIN="${TDX_IMX_HAB_CST_BIN}" \
        TDX_IMX_HAB_CST_ARGS="${TDX_IMX_HAB_CST_ARGS}" \
        IMXBOOT="${SPL_BINARY}" \
        HAB_LOG="${SPL_BINARY}.log" \
        ${WORKDIR}/imx6_imx7_create_csf.sh -m "${soc}" -c "csf_SPL"

        bbdebug 1 "Concatenating CSF binary to the U-Boot SPL"
        # Save unsigned SPL and replace original with signed one:
        mv "${SPL_BINARY}" "${SPL_BINARY}-unsigned"
        cat "${SPL_BINARY}-unsigned" "csf_SPL.bin" > "${SPL_BINARY}"

        # TODO: Regenerate u-boot-with-spl.imx
    fi

    # Create fuse commands
    create_fuse_cmds_helper "${soc}"
}

# Sign U-Boot binary and/or SPL (this works with i.MX8M).
# This is supposed to run in the U-Boot build directory.
imx8m_sign_habv4() {
    local soc="$1"
    local type="$2"
    local binary="$3"

    if [ ! -e "${TDX_IMX_HAB_CST_BIN}" ]; then
        bbfatal "Could not find CST binary at ${TDX_IMX_HAB_CST_BIN}."
    fi

    # Save unsigned image
    cp "flash.bin" "flash.bin-unsigned"

    # Sign the bootloader container (in-place)
    LIBFAKETIME_PATH="${STAGING_LIBDIR_NATIVE}/faketime/libfaketime.so.1" \
    TDX_IMX_HAB_CST_SRK="${TDX_IMX_HAB_CST_SRK}" \
    TDX_IMX_HAB_CST_SRK_CERT="${TDX_IMX_HAB_CST_SRK_CERT}" \
    TDX_IMX_HAB_CST_CSF_CERT="${TDX_IMX_HAB_CST_CSF_CERT}" \
    TDX_IMX_HAB_CST_IMG_CERT="${TDX_IMX_HAB_CST_IMG_CERT}" \
    TDX_IMX_HAB_CST_BIN="${TDX_IMX_HAB_CST_BIN}" \
    TDX_IMX_HAB_CST_ARGS="${TDX_IMX_HAB_CST_ARGS}" \
    TDX_IMX_HAB_CST_HSM="${TDX_SIGNED_HSM}" \
    TDX_IMX_HAB_CST_SRK_CA="${TDX_IMX_HAB_CST_SRK_CA}" \
    TDX_IMX_HAB_CST_SRK_INDEX="${TDX_IMX_HAB_CST_SRK_INDEX}" \
    ${WORKDIR}/imx8m_sign.sh

    # Create fuse commands
    create_fuse_cmds_helper "${soc}"
}

uboot_assemble_fitimage_helper:append:mx6dl-generic-bsp() {
    type="$1"
    binary="$2"
    imx6_imx7_sign_habv4 "IMX6" "${type}" "${binary}"
}

uboot_assemble_fitimage_helper:append:mx6q-generic-bsp() {
    type="$1"
    binary="$2"
    imx6_imx7_sign_habv4 "IMX6" "${type}" "${binary}"
}

uboot_assemble_fitimage_helper:append:mx6ull-generic-bsp() {
    type="$1"
    binary="$2"
    imx6_imx7_sign_habv4 "IMX6ULL" "${type}" "${binary}"
}

uboot_assemble_fitimage_helper:append:mx7-generic-bsp() {
    type="$1"
    binary="$2"
    imx6_imx7_sign_habv4 "IMX7" "${type}" "${binary}"
}

uboot_assemble_fitimage_helper:append:mx8m-generic-bsp() {
    type="$1"
    binary="$2"
    imx8m_sign_habv4 "IMX8M" "${type}" "${binary}"
}

do_deploy:append() {
    if [ -e ${WORKDIR}/fuse-cmds.txt ]; then
        install -d ${DEPLOYDIR}
        install -m 0644 ${WORKDIR}/fuse-cmds.txt ${DEPLOYDIR}
        install -m 0644 ${WORKDIR}/imx-config.fuse ${DEPLOYDIR}
    fi
}
