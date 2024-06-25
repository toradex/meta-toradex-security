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
    echo "    TDX_IMX_HAB_CST_CSF_CERT  Path to CSFK Certificate,         e.g. CSF1_1_sha256_4096_65537_v3_usr_crt.pem"
    echo "    TDX_IMX_HAB_CST_IMG_CERT  Path to Public key certificate,   e.g. IMG1_1_sha256_4096_65537_v3_usr_crt.pem"
    echo "    TDX_IMX_HAB_CST_BIN       Path to NXP CST Binary            e.g. cst-3.1.0/release/linux64/bin/cst"
    echo "    IMXBOOT                   Path to unsigned imx-boot image,  e.g. u-boot.imx"
    echo "    HAB_LOG                   Path to u-boot build log file containing hab info"
    echo
    echo " required arguments:"
    echo "    -m --machine              target SoC (i.e IMX6, IMX7, IMX6ULL)"
    echo "    -c --csf                  CSF basename (outputs will be <basename>.{csf,bin,log})"
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
            -m|--machine)
                MACHINE="$2"
                shift # past argument
                shift # past value
            ;;
            -c|--csf)
                CSF="$2"
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
        help "Could not find '$1'"
    fi
	echo "Verified $2=$1"
}

set_template_file() {
    case ${MACHINE} in
        "IMX6ULL")
            TEMPLATE_FILE="imx6ull_template.csf"
	    ;;
	"IMX7")
	    TEMPLATE_FILE="imx7_template.csf"
	    ;;
	"IMX6")
            TEMPLATE_FILE="imx6_template.csf"
	    ;;
        *)
	    echo "Invalid SoC!"
	    return 1
	    ;;
    esac
}

generate_csf() {
    # Copy template file
    cp ${DIR_SCRIPT}/${TEMPLATE_FILE} ${CSF}.csf

    # Delete 'Blocks =' section from template
    sed -i '/Blocks = /d' ${CSF}.csf

    # Update Key Locations
    sed -i "s|CST_SRK|${TDX_IMX_HAB_CST_SRK}|g" ${CSF}.csf
    sed -i "s|CST_CSF_CERT|${TDX_IMX_HAB_CST_CSF_CERT}|g" ${CSF}.csf
    sed -i "s|CST_IMG_CERT|${TDX_IMX_HAB_CST_IMG_CERT}|g" ${CSF}.csf

    # Append Blocks
    echo "    Blocks = $(grep 'HAB Blocks' ${HAB_LOG} | awk '{print $3, $4, $5}') \"${IMXBOOT}\"" >> ${CSF}.csf

    # Generate Binary
    ${TDX_IMX_HAB_CST_BIN} -i ${CSF}.csf -o ${CSF}.bin > ${CSF}.log 2>&1
    cat ${CSF}.log
}

parse_args "$@"

# Print command for Yocto logs
echo $0 "$@"

# Verify required variables
[ -n "${MACHINE}" ] || help "machine name not specified"
[ -n "${CSF}" ]     || help "CSF basename not specified"

set_template_file

verify_env "${TDX_IMX_HAB_CST_SRK}" "TDX_IMX_HAB_CST_SRK"
verify_env "${TDX_IMX_HAB_CST_CSF_CERT}" "TDX_IMX_HAB_CST_CSF_CERT"
verify_env "${TDX_IMX_HAB_CST_IMG_CERT}" "TDX_IMX_HAB_CST_IMG_CERT"
verify_env "${TDX_IMX_HAB_CST_BIN}" "TDX_IMX_HAB_CST_BIN"
verify_env "${IMXBOOT}" "IMXBOOT"
verify_env "${HAB_LOG}" "HAB_LOG"

generate_csf
