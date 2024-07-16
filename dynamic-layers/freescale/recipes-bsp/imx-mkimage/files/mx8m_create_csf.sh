#!/bin/bash

set -e

readonly FILE_SCRIPT="$(basename "$0")"
readonly DIR_SCRIPT="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"

help() {
    if [ -n "${1}" ]; then
        echo
        echo "***"
        echo " ERROR: ${1}"
        echo "***"
    fi
    echo
    echo " Usage: ${DIR_SCRIPT}/${FILE_SCRIPT} <options>"
    echo
    echo " Required Environment Variables:"
    echo
    echo "    TDX_IMX_HAB_CST_SRK       Path to SRK Table"
    echo "                              e.g. SRK_1_2_3_4_table.bin"
    echo "    TDX_IMX_HAB_CST_SRK_CERT  Path to SRK public key certificate"
    echo "                              e.g. SRK1_sha256_2048_65537_v3_ca_crt.pem (when CA flag is set)"
    echo "                              e.g. SRK1_sha256_2048_65537_v3_usr_crt.pem (when CA flag is not set)"
    echo "    TDX_IMX_HAB_CST_CSF_CERT  Path to CSF public key certificate (needed only if CA flag is set for SRK)"
    echo "                              e.g. CSF1_1_sha256_2048_65537_v3_usr_crt.pem"
    echo "    TDX_IMX_HAB_CST_IMG_CERT  Path to IMG public key certificate (needed only if CA flag is set for SRK)"
    echo "                              e.g. IMG1_1_sha256_2048_65537_v3_usr_crt.pem"
    echo "    TDX_IMX_HAB_CST_BIN       Path to NXP CST Binary            e.g. cst-3.1.0/release/linux64/bin/cst"
    echo "    IMXBOOT                   Path to unsigned imx-boot image,  e.g. imx-boot-imx8mn-var-som-sd.bin-flash_ddr4_evk"
    echo "    LOG_MKIMAGE               Path to mkimage log file"
    echo "    LOG_PRINT_FIT_HAB         Path to mkimage print_fit_hab log file"
    echo
    echo " required arguments:"
    echo "    -t --target                imx-boot target, e.g. imx8mn-var-som-sd.bin-flash_ddr4_evk"
    echo
    echo " optional:"
    echo "    -h --help            display this Help message"
    echo
    exit 1
}

parse_args() {
    while [[ $# -gt 0 ]]; do
        case $1 in
            -h|--help)
                help
            ;;
            -t|--target)
                TARGET="$2"
                shift # past argument
                shift # past value
            ;;
            *)    # unknown option
                echo "Unknown option: $1"
                help
            ;;
        esac
    done
}

# Verify environment variable is set and file exists
check_fileref() {
    if [ -z "$1" ]; then
        help "Please set environment variable '$2'"
    fi
    if [ ! -f "$1" ]; then
        help "Could not find '$1'"
    fi
    echo "Verified $2=$1"
}

validate_environ() {
    if [ -z "${TARGET}" ]; then
        help "target argument required"
    fi
    check_fileref "${TDX_IMX_HAB_CST_SRK}" "TDX_IMX_HAB_CST_SRK"
    check_fileref "${TDX_IMX_HAB_CST_SRK_CERT}" "TDX_IMX_HAB_CST_SRK_CERT"
    if [ "${TDX_IMX_HAB_CST_SRK_CERT##*_ca_}" = "crt.pem" ]; then
        check_fileref "${TDX_IMX_HAB_CST_CSF_CERT}" "TDX_IMX_HAB_CST_CSF_CERT"
        check_fileref "${TDX_IMX_HAB_CST_IMG_CERT}" "TDX_IMX_HAB_CST_IMG_CERT"
    fi
    check_fileref "${TDX_IMX_HAB_CST_BIN}" "TDX_IMX_HAB_CST_BIN"
    check_fileref "${IMXBOOT}" "IMXBOOT"
    check_fileref "${LOG_MKIMAGE}" "LOG_MKIMAGE"
    check_fileref "${LOG_PRINT_FIT_HAB}" "LOG_PRINT_FIT_HAB"
}

# $1: CSF file path/name w/o extension
# $2: SPL flag (1 or 0)
generate_csf_common() {
    local image_csf="${1?CSF file name expected}.csf"
    local spl="${2?SPL flag expected}"

    # Copy template file
    echo "Creating CSF file: ${image_csf}"
    cp "${DIR_SCRIPT}/mx8m_template.csf" "${image_csf}"

    # Determine key index (use file name)
    local kidx
    kidx=${TDX_IMX_HAB_CST_SRK_CERT##*/}
    kidx=${kidx##SRK}
    kidx=${kidx%%_*}
    if [ "${#kidx}" != 1 ]; then
        echo "Certificate file name (defined by TDX_IMX_HAB_CST_SRK_CERT) does" \
             "not match expected pattern - could not determine key index."
        exit 1
    fi
    kidx=$((kidx - 1))

    # Determine whether or not the CA flag was set
    local ca
    if [ "${TDX_IMX_HAB_CST_SRK_CERT##*_ca_}" = "crt.pem" ]; then
        ca=1
    elif [ "${TDX_IMX_HAB_CST_SRK_CERT##*_usr_}" = "crt.pem" ]; then
        ca=0
    else
        echo "Certificate file name (defined by TDX_IMX_HAB_CST_SRK_CERT)" \
             "does not match expected pattern - could not determine if CA flag is set."
        exit 1
    fi

    # Update "Install SRK" section
    sed -i "s|@@CST_SRK@@|${TDX_IMX_HAB_CST_SRK}|g" "${image_csf}"
    sed -i "s|@@CST_KIDX@@|${kidx}|g" "${image_csf}"

    if [ "$ca" = 1 ]; then
        # Keep "Install CSFK" section and update its contents
        sed -i "/#+START_INSTALL_CSFK_BLOCK/d; /#+END_INSTALL_CSFK_BLOCK/d" "${image_csf}"
        sed -i "s|@@CST_CSF_CERT@@|${TDX_IMX_HAB_CST_CSF_CERT}|g" "${image_csf}"
        # Delete "Install NOCAK" section
        sed -i "/#+START_INSTALL_NOCAK_BLOCK/,/#+END_INSTALL_NOCAK_BLOCK/d" "${image_csf}"
    else
        # Delete "Install CSFK" section
        sed -i "/#+START_INSTALL_CSFK_BLOCK/,/#+END_INSTALL_CSFK_BLOCK/d" "${image_csf}"
        # Keep "Install NOCAK" section and update its contents
        sed -i "/#+START_INSTALL_NOCAK_BLOCK/d; /#+END_INSTALL_NOCAK_BLOCK/d" "${image_csf}"
        sed -i "s|@@CST_SRK_CERT@@|${TDX_IMX_HAB_CST_SRK_CERT}|g" "${image_csf}"
    fi

    if [ "$spl" =  1 ]; then
        # Keep "Unlock" section
        sed -i "/#+START_UNLOCK_BLOCK/d; /#+END_UNLOCK_BLOCK/d" "${image_csf}"
    else
        # Delete "Unlock" section
        sed -i "/#+START_UNLOCK_BLOCK/,/#+END_UNLOCK_BLOCK/d" "${image_csf}"
    fi

    if [ "$ca" = 1 ]; then
        # Keep "Install Key" section and update its contents
        sed -i "/#+START_INSTALL_KEY_BLOCK/d; /#+END_INSTALL_KEY_BLOCK/d" "${image_csf}"
        sed -i "s|@@CST_IMG_CERT@@|${TDX_IMX_HAB_CST_IMG_CERT}|g" "${image_csf}"
        # Update part of "Authenticate Data" section
        # Verification index is 2 (IMGK slot)
        sed -i "s|@@CST_AUTH_KIDX@@|2|g" "${image_csf}"
    else
        # Delete "Install Key" section
        sed -i "/#+START_INSTALL_KEY_BLOCK/,/#+END_INSTALL_KEY_BLOCK/d" "${image_csf}"
        # Update part of "Authenticate Data" section
        # Verification index is 0 (SRK slot)
        sed -i "s|@@CST_AUTH_KIDX@@|0|g" "${image_csf}"
    fi
}

generate_csf_spl() {
    generate_csf_common "${CSF_SPL}" "1"

    # Update "Blocks" data in "Authenticate Data" section
    sed -i '/Blocks = /d' "${CSF_SPL}.csf"
    echo "    Blocks = $(grep 'spl hab block' "${LOG_MKIMAGE}" | awk '{print $4, $5, $6}') \"${IMXBOOT}\"" >> "${CSF_SPL}.csf"

    # Generate Binary
    "${TDX_IMX_HAB_CST_BIN}" -i "${CSF_SPL}.csf" -o "${CSF_SPL}.bin" > "${CSF_SPL}.log" 2>&1
    cat "${CSF_SPL}.log"
}

generate_csf_fit() {
    generate_csf_common "${CSF_FIT}" "0"

    # Update "Blocks" data in "Authenticate Data" section

    # Delete 'Blocks =' section from template
    sed -i '/Blocks = /d' "${CSF_FIT}.csf"

    # Append Blocks

    # Append block from mkimage log
    echo "    Blocks = $(grep 'sld hab block' "${LOG_MKIMAGE}" | awk '{print $4, $5, $6}') \"${IMXBOOT}\", \\" >> "${CSF_FIT}.csf"

    # Append blocks from mkimage print_fit_hab
    # It looks like this, with a variable number of lines after TEE_LOAD_ADDR....
    # TEE_LOAD_ADDR=0xbe000000 ATF_LOAD_ADDR=0x00920000 VERSION=v1 ./print_fit_hab.sh 0x60000 imx8mm-var-dart-customboard.dtb imx8mm-var-som-symphony.dtb
    # 0x40200000 0x5AC00 0xA8F90
    # 0x402A8F90 0x103B90 0x7942
    # 0x402B08D2 0x10B4D4 0x7AEA
    # 0x920000 0x112FC0 0xA1E0

    # Read to end of file
    BLOCKS_RAW="$(sed -n '/TEE_LOAD_ADDR=/,$p' "${LOG_PRINT_FIT_HAB}")"
    # Split each newline into array
    readarray -t BLOCKS <<<"$BLOCKS_RAW"
    # Remove first line
    unset "BLOCKS[0]"
    # Loop through each line
    PREFIX=""
    for BLOCK in "${BLOCKS[@]}"; do
        printf "${PREFIX}             ${BLOCK} \"${IMXBOOT}\"" >> "${CSF_FIT}.csf"
        PREFIX=", \\ \n"
    done
    echo "" >> "${CSF_FIT}.csf"

    # Generate Binary
    "${TDX_IMX_HAB_CST_BIN}" -i "${CSF_FIT}.csf" -o "${CSF_FIT}.bin" > "${CSF_FIT}.log" 2>&1
    cat "${CSF_FIT}.log"
}

parse_args "$@"

# Print command for Yocto logs
echo "$0" "$@"

readonly CSF_SPL="${TARGET}-csf-spl"
readonly CSF_FIT="${TARGET}-csf-fit"

cd "${DIR_SCRIPT}"

validate_environ
generate_csf_spl
generate_csf_fit
