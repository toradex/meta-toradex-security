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

python __anonymous() {
    scfgs = (d.getVar('TDX_IMX_HAB_SDP_CONFIGS_INTERNAL') or "").split()
    ucfgs = (d.getVar('UBOOT_CONFIG') or "").split()
    if not ucfgs:
        for _scfg in scfgs:
            if _scfg != "*":
                bb.warnonce(
                    'UBOOT_CONFIG is not in use; TDX_IMX_HAB_SDP_CONFIGS should '
                    'be either empty or set to "*", but it instead contains a '
                    f'configuration named "{_scfg}"; bootloader will not be '
                    'built for SDP loading.')
                break
        return

    for _scfg in scfgs:
        if _scfg == "*":
            continue
        if _scfg not in ucfgs:
            bb.warnonce(
                f'Configuration named "{_scfg}" in TDX_IMX_HAB_SDP_CONFIGS '
                'is not listed in UBOOT_CONFIG and will be ignored; your '
                'bootloader may not be built for SDP loading.')
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

    binary_to_sign="${UBOOT_BINARY_HAB}"
    binary_to_mkcsf="${UBOOT_BINARY_HAB}"

    csf_inc_dcd="0"
    # Run loop in a subshell with set -f to avoid expansion of "*", which
    # variable TDX_IMX_HAB_SDP_CONFIGS (and the internal one) may contain.
    (
	set -f
	for _config in ${TDX_IMX_HAB_SDP_CONFIGS_INTERNAL}; do
            if [ "${_config}" = "${type}" ] || [ "${_config}" = "*" ]; then
		exit 0
            fi
	done
	exit 1
    ) && csf_inc_dcd="1"

    # For SDP loading we need:
    #
    # (a) a CSF determined from a bootloader with a cleared DCD pointer; and
    # (b) a CSF that covers (protects) the DCD blocks.
    #
    if [ "${csf_inc_dcd}" = "1" ]; then
        bbdebug 1 "Generating bootloader binary with cleared DCD pointer (type='${type}')"
        if [ "$(xxd -p -l1 "${binary_to_sign}")" != "d1" ]; then
            bbfatal "File '${binary_to_sign}' does not seem to start with an IVT; aborting."
        fi
        binary_to_mkcsf="${UBOOT_BINARY_HAB}-clrdcd"
        cp -v "${binary_to_sign}" "${binary_to_mkcsf}"
        cp -v "${binary_to_sign}.log" "${binary_to_mkcsf}.log"
        # As per IMX6DRM/IMX7DRM, section titled "Image vector table structure",
        # the 32-bit DCD pointer is at offset 12 (0xC); clear it.
        printf '\0\0\0\0' | dd of="${binary_to_mkcsf}" bs=1 seek=12 conv=notrunc
    else
        bbdebug 1 "DCD pointer clearing has been skipped"
    fi

    bbdebug 1 "Generating CSF from U-Boot binary '${binary_to_mkcsf}' (type='${type}')"
    # Generate CSF file:
    LIBFAKETIME_PATH="${STAGING_LIBDIR_NATIVE}/faketime/libfaketime.so.1" \
    TDX_IMX_HAB_CST_SRK="${TDX_IMX_HAB_CST_SRK}" \
    TDX_IMX_HAB_CST_SRK_CERT="${TDX_IMX_HAB_CST_SRK_CERT}" \
    TDX_IMX_HAB_CST_CSF_CERT="${TDX_IMX_HAB_CST_CSF_CERT}" \
    TDX_IMX_HAB_CST_IMG_CERT="${TDX_IMX_HAB_CST_IMG_CERT}" \
    TDX_IMX_HAB_CST_BIN="${TDX_IMX_HAB_CST_BIN}" \
    TDX_IMX_HAB_CST_ARGS="${TDX_IMX_HAB_CST_ARGS}" \
    CSF_INC_DCD="${csf_inc_dcd}" \
    IMXBOOT="${binary_to_mkcsf}" \
    HAB_LOG="${binary_to_mkcsf}.log" \
    ${WORKDIR}/imx6_imx7_create_csf.sh -m "${soc}" -c "csf_uboot"

    # Save unsigned image and generate signed version by concatenating with the CSF.
    bbdebug 1 "Concatenating CSF binary to U-Boot binary '${binary_to_sign}'"
    if [ -n "${type}" ]; then
        cp  "${binary_to_sign}" "${UBOOT_BINARYNAME}-${type}.${UBOOT_SUFFIX}-unsigned"
        cat "${binary_to_sign}" "csf_uboot.bin" > "${UBOOT_BINARYNAME}-${type}.${UBOOT_SUFFIX}"
    else
        mv  "${binary_to_sign}" "${UBOOT_BINARY}-unsigned"
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
    ${STAGING_BINDIR_NATIVE}/create_fuse_cmds.sh \
        "${soc}" \
        "${TDX_IMX_HAB_CST_SRK_FUSE}" \
        "${WORKDIR}/fuse-cmds.txt" \
        "${RECIPE_SYSROOT_NATIVE}${datadir}/imx-fuses"
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
    ${STAGING_BINDIR_NATIVE}/create_fuse_cmds.sh \
        "${soc}" \
        "${TDX_IMX_HAB_CST_SRK_FUSE}" \
        "${WORKDIR}/fuse-cmds.txt" \
        "${RECIPE_SYSROOT_NATIVE}${datadir}/imx-fuses"
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
