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
    echo "    TDX_IMX_HAB_CST_SGK_CERT  Path to SGK public key certificate (needed only if CA flag is set for SRK)"
    echo "                              e.g. SGK1_1_sha256_2048_65537_v3_usr_crt.pem"
    echo "    TDX_IMX_HAB_CST_BIN       Path to NXP CST Binary            e.g. cst-3.1.0/release/linux64/bin/cst"
    echo "    UNSIGNED_IMAGE            Path to unsigned image"
    echo "    LOG_MKIMAGE               Path to mkimage log file"
    echo
    echo " required arguments:"
    echo "    -t --target               imx-boot target, e.g. imx8mn-var-som-sd.bin-flash_ddr4_evk"
    echo
    echo " optional:"
    echo "    -h --help                 display this help message"
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
        check_fileref "${TDX_IMX_HAB_CST_SGK_CERT}" "TDX_IMX_HAB_CST_SGK_CERT"
    fi
    check_fileref "${TDX_IMX_HAB_CST_BIN}" "TDX_IMX_HAB_CST_BIN"
    check_fileref "${UNSIGNED_IMAGE}" "UNSIGNED_IMAGE"
    check_fileref "${LOG_MKIMAGE}" "LOG_MKIMAGE"
}

generate_csf_ahab() {
    local image_csf="${DIR_SCRIPT}/${TARGET}.csf"

    # Copy template file
    echo "Creating CSF file: ${image_csf}"
    cp "${DIR_SCRIPT}/mx8_template.csf" "${image_csf}"

    # Get offset from log
    local header block
    header=$(grep "CST: CONTAINER 0 offset:" "${LOG_MKIMAGE}" | tail -1 | awk '{print $5}')
    block=$(grep "CST: CONTAINER 0: Signature Block" "${LOG_MKIMAGE}" | tail -1 | awk '{print $9}')

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

    # Update SRK files
    sed -i "s|@@CST_SRK@@|${TDX_IMX_HAB_CST_SRK}|g" "${image_csf}"
    sed -i "s|@@CST_KEY@@|${TDX_IMX_HAB_CST_SRK_CERT}|g" "${image_csf}"
    sed -i "s|@@CST_KIDX@@|${kidx}|g" "${image_csf}"

    if [ "${TDX_IMX_HAB_CST_SRK_CERT##*_ca_}" = "crt.pem" ]; then
        # File name ends with "_ca_crt.pem": an SGK is expected.
        local sgk="${TDX_IMX_HAB_CST_SGK_CERT?TDX_IMX_HAB_CST_SGK_CERT must be set}"
        echo "Inferred that CA flag was set; signing with SRK and SGK."
        sed -i "/#+START_SGK_BLOCK/d; /#+END_SGK_BLOCK/d" "${image_csf}"
        sed -i "s|@@CST_SGK@@|${sgk}|g" "${image_csf}"

    elif [ "${TDX_IMX_HAB_CST_SRK_CERT##*_usr_}" = "crt.pem" ]; then
        # File name ends with "_usr_crt.pem": an SGK is not expected.
        # Remove SGK block from CSF.
        echo "Inferred that CA flag was not set; signing with SRK only."
        sed -i "/#+START_SGK_BLOCK/,/#+END_SGK_BLOCK/d" "${image_csf}"
    else
        # File name does not follow expected pattern.
        echo "Certificate file name (defined by TDX_IMX_HAB_CST_SRK_CERT)" \
             "does not match expected pattern - could not determine if CA flag is set."
        exit 1
    fi

    # Update offset
    sed -i "s|@@FLASH.BIN@@|${UNSIGNED_IMAGE}|g" "${image_csf}"
    sed -i "s|@@OFFSETS_ROW:.*$|Offsets = ${header} ${block}|g" "${image_csf}"

    echo "Signing '${UNSIGNED_IMAGE}' with CST tool."
    echo "Tool location: '${TDX_IMX_HAB_CST_BIN}'"
    echo "CSF location: '${image_csf}'"

    # Sign
    "${TDX_IMX_HAB_CST_BIN}" -i "${image_csf}" -o "${UNSIGNED_IMAGE}-signed"
}

parse_args "$@"

# Print command for Yocto logs
echo "$0" "$@"

cd "${DIR_SCRIPT}"

validate_environ
generate_csf_ahab
