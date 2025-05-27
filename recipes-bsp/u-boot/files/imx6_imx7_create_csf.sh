#!/bin/bash

[ "${XTRACE}" = "1" ] && set -x

set -e

# shellcheck disable=SC2155
readonly FILE_SCRIPT="$(basename "$0")"
# shellcheck disable=SC2155
readonly DIR_SCRIPT="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"

MACHINE=""
CSF=""
TEMPLATE_FILE=""

error() {
    echo "***" >&2
    echo " ERROR: ${1}" >&2
    echo "***" >&2
    exit 1
}

help() {
    echo
    echo " Usage: ${DIR_SCRIPT}/${FILE_SCRIPT} <options>"
    echo
    echo " Required Environment Variables:"
    echo
    echo "    TDX_IMX_HAB_CST_BIN       Path to NXP CST Binary"
    echo "                              e.g. cst-3.1.0/release/linux64/bin/cst"
    echo "    TDX_IMX_HAB_CST_SRK       Path to SRK Table"
    echo "                              e.g. SRK_1_2_3_4_table.bin"
    echo "    TDX_IMX_HAB_CST_SRK_CERT  Path to SRK public key certificate"
    echo "                              e.g. SRK1_sha256_2048_65537_v3_ca_crt.pem (when CA flag is set)"
    echo "                              e.g. SRK1_sha256_2048_65537_v3_usr_crt.pem (when CA flag is not set)"
    echo "    TDX_IMX_HAB_CST_CSF_CERT  Path to CSF public key certificate (needed only if CA flag is set for SRK)"
    echo "                              e.g. CSF1_1_sha256_2048_65537_v3_usr_crt.pem"
    echo "    TDX_IMX_HAB_CST_IMG_CERT  Path to IMG public key certificate (needed only if CA flag is set for SRK)"
    echo "                              e.g. IMG1_1_sha256_2048_65537_v3_usr_crt.pem"
    echo "    IMXBOOT                   Path to unsigned imx-boot image"
    echo "                              e.g. u-boot.imx"
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

# Verify that environment variable is set and file exists.
check_fileref() {
    local file="${1?file name expected}"
    local varname="${2?variable name expected}"
    if [ -z "${file}" ]; then
        error "Please set environment variable '${varname}'"
    fi
    if [ ! -f "${file}" ]; then
        error "Could not find '${file}' referenced by variable ${varname} (CWD=$(pwd))"
    fi
    echo "Verified ${varname}=${file}"
}

# Verify all relevant environment variables.
validate_environ() {
    check_fileref "${TDX_IMX_HAB_CST_BIN}" "TDX_IMX_HAB_CST_BIN"
    check_fileref "${TDX_IMX_HAB_CST_SRK}" "TDX_IMX_HAB_CST_SRK"
    check_fileref "${TDX_IMX_HAB_CST_SRK_CERT}" "TDX_IMX_HAB_CST_SRK_CERT"
    if [ "${TDX_IMX_HAB_CST_SRK_CERT##*_ca_}" = "crt.pem" ]; then
        check_fileref "${TDX_IMX_HAB_CST_CSF_CERT}" "TDX_IMX_HAB_CST_CSF_CERT"
        check_fileref "${TDX_IMX_HAB_CST_IMG_CERT}" "TDX_IMX_HAB_CST_IMG_CERT"
    fi
    check_fileref "${IMXBOOT}" "IMXBOOT"
    check_fileref "${HAB_LOG}" "HAB_LOG"
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
    local image_csf="${CSF}.csf"

    # Copy template file
    echo "Creating CSF file: ${image_csf}"
    cp "${DIR_SCRIPT}/${TEMPLATE_FILE}" "${image_csf}"

    # Determine key index (use file name):
    local kidx
    kidx=${TDX_IMX_HAB_CST_SRK_CERT##*/}
    kidx=${kidx##SRK}
    kidx=${kidx%%_*}
    if [ "${#kidx}" != 1 ]; then
        echo "Certificate file name (defined by TDX_IMX_HAB_CST_SRK_CERT) does" \
             "not match expected pattern - could not determine SRK key index."
        exit 1
    fi

    if [ "${kidx}" -ge 1 ] && [ "${kidx}" -le 4 ]; then
        echo "Using SRK${kidx} for signing."
    else
        echo "Bad SRK key index '${kidx}' (inferred from TDX_IMX_HAB_CST_SRK_CERT) - aborting."
        exit 1
    fi
    kidx=$((kidx - 1))

    # Determine whether or not the CA flag was set:
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

    # Update "Install SRK" section:
    sed -i "s|@@CST_SRK@@|${TDX_IMX_HAB_CST_SRK}|g" "${image_csf}"
    sed -i "s|@@CST_KIDX@@|${kidx}|g" "${image_csf}"

    if [ "$ca" = 1 ]; then
        # Keep "Install CSFK" section and update its contents:
        sed -i "/#+START_INSTALL_CSFK_BLOCK/d; /#+END_INSTALL_CSFK_BLOCK/d" "${image_csf}"
        sed -i "s|@@CST_CSF_CERT@@|${TDX_IMX_HAB_CST_CSF_CERT}|g" "${image_csf}"
        # Delete "Install NOCAK" section:
        sed -i "/#+START_INSTALL_NOCAK_BLOCK/,/#+END_INSTALL_NOCAK_BLOCK/d" "${image_csf}"
    else
        # Delete "Install CSFK" section:
        sed -i "/#+START_INSTALL_CSFK_BLOCK/,/#+END_INSTALL_CSFK_BLOCK/d" "${image_csf}"
        # Keep "Install NOCAK" section and update its contents:
        sed -i "/#+START_INSTALL_NOCAK_BLOCK/d; /#+END_INSTALL_NOCAK_BLOCK/d" "${image_csf}"
        sed -i "s|@@CST_SRK_CERT@@|${TDX_IMX_HAB_CST_SRK_CERT}|g" "${image_csf}"
    fi

    # TODO: Consider handling the Unlock section.
    # if [ "$spl" =  1 ]; then
    #     # Keep "Unlock" section:
    #     sed -i "/#+START_UNLOCK_BLOCK/d; /#+END_UNLOCK_BLOCK/d" "${image_csf}"
    # else
    #     # Delete "Unlock" section:
    #     sed -i "/#+START_UNLOCK_BLOCK/,/#+END_UNLOCK_BLOCK/d" "${image_csf}"
    # fi

    if [ "$ca" = 1 ]; then
        # Keep "Install Key" section and update its contents:
        sed -i "/#+START_INSTALL_KEY_BLOCK/d; /#+END_INSTALL_KEY_BLOCK/d" "${image_csf}"
        sed -i "s|@@CST_IMG_CERT@@|${TDX_IMX_HAB_CST_IMG_CERT}|g" "${image_csf}"
        # Update part of "Authenticate Data" section:
        # Verification index is 2 (IMGK slot).
        sed -i "s|@@CST_AUTH_KIDX@@|2|g" "${image_csf}"
    else
        # Delete "Install Key" section
        sed -i "/#+START_INSTALL_KEY_BLOCK/,/#+END_INSTALL_KEY_BLOCK/d" "${image_csf}"
        # Update part of "Authenticate Data" section:
        # Verification index is 0 (SRK slot).
        sed -i "s|@@CST_AUTH_KIDX@@|0|g" "${image_csf}"
    fi

    # Delete 'Blocks =' section from template
    sed -i "/Blocks = /d" "${image_csf}"

    # Append Blocks
    echo "    Blocks = $(grep 'HAB Blocks' "${HAB_LOG}" | awk '{print $3, $4, $5}') \"${IMXBOOT}\"" >> "${image_csf}"

    # Generate Binary
    if ! ${TDX_IMX_HAB_CST_BIN} -i "${image_csf}" -o "${CSF}.bin" > "${CSF}.log" 2>&1; then
        echo "CST execution log:" >&2
        cat "${CSF}.log" | sed 's@^@|@' >&2
        error "CST failed to execute; please check logs."
    fi
    cat "${CSF}.log"
}

parse_args "$@"

# Print command for Yocto logs
echo "$0" "$@"

# Verify required variables
[ -n "${MACHINE}" ] || error "machine name not specified"
[ -n "${CSF}" ]     || error "CSF basename not specified"

set_template_file
validate_environ
generate_csf
