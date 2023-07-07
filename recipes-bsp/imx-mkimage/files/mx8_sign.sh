#!/bin/bash

set -e

readonly FILE_SCRIPT="$(basename "$0")"
readonly DIR_SCRIPT="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"

help() {
    if [ -n "${1}" ]; then
        echo " Error: ${1}"
    fi
    echo
    echo " Usage: ${DIR_SCRIPT}/${FILE_SCRIPT} <options>"
    echo
    echo " Required Environment Variables:"
    echo
    echo "    TDX_IMX_HAB_CST_SRK       Path to SRK Table,                e.g. SRK_1_2_3_4_table.bin"
    echo "    TDX_IMX_HAB_CST_SRK_CERT  Path to Public key certificate,   e.g. SRK1_sha256_2048_65537_v3_usr_crt.pem"
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
verify_env() {
    if [ -z "$1" ]; then
        help "Please set environment variable '$2'"
    fi
    if [ ! -f $1 ]; then
        help "Could not find '$1' $(pwd)"
    fi
	echo "Verified $2=$1"
}

generate_csf_ahab() {
    CST_DIR=${TDX_IMX_HAB_CST_BIN%/*}
    IMAGE_CSF=${CST_DIR}/${TARGET}.csf

    # Copy template file
    cp ${DIR_SCRIPT}/mx8_template.csf ${IMAGE_CSF}

    # Get offset from log
    HEADER=$(cat ${LOG_MKIMAGE} | grep "CST: CONTAINER 0 offset:" | tail -1 | awk '{print $5}')
    BLOCK=$(cat ${LOG_MKIMAGE} | grep "CST: CONTAINER 0: Signature Block" | tail -1 | awk '{print $9}')

    # Update SRK files
    sed -i "s|CST_SRK|${TDX_IMX_HAB_CST_SRK}|g" ${IMAGE_CSF}
    sed -i "s|CST_KEY|${TDX_IMX_HAB_CST_SRK_CERT}|g" ${IMAGE_CSF}

    # Update offset
    sed -i "s|flash.bin|${UNSIGNED_IMAGE}|g" ${IMAGE_CSF}
    sed -i '/Offsets   = 0x400/d' ${IMAGE_CSF}
    echo "Offsets = ${HEADER} ${BLOCK}" >> ${IMAGE_CSF}

    # Sign
    ${TDX_IMX_HAB_CST_BIN} -i ${IMAGE_CSF} -o ${UNSIGNED_IMAGE}-signed
}

parse_args "$@"

# Print command for Yocto logs
echo $0 "$@"

# Verify required variables
if [ -z "${TARGET}" ]; then
    help "target argument required"
fi
verify_env "${TDX_IMX_HAB_CST_SRK}" "TDX_IMX_HAB_CST_SRK"
verify_env "${TDX_IMX_HAB_CST_SRK_CERT}" "TDX_IMX_HAB_CST_SRK_CERT"
verify_env "${TDX_IMX_HAB_CST_BIN}" "TDX_IMX_HAB_CST_BIN"
verify_env "${UNSIGNED_IMAGE}" "UNSIGNED_IMAGE"
verify_env "${LOG_MKIMAGE}" "LOG_MKIMAGE"

cd ${DIR_SCRIPT}

generate_csf_ahab
