#!/bin/bash

[ "${XTRACE}" = "1" ] && set -x

set -e

# shellcheck disable=SC2155
readonly FILE_SCRIPT="$(basename "$0")"
# shellcheck disable=SC2155
readonly DIR_SCRIPT="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"

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
    echo "    TDX_IMX_HAB_CST_SGK_CERT  Path to SGK public key certificate (needed only if CA flag is set for SRK)"
    echo "                              e.g. SGK1_1_sha256_2048_65537_v3_usr_crt.pem"
    echo "    TDX_IMX_HAB_CST_HSM       Enable usage of HSM (via PKCS11) when set to 1"
    echo "    TDX_IMX_HAB_CST_SRK_CA    Whether or not the SRK certificates have the CA flag"
    echo "                              Only used when HSM is enabled (TDX_IMX_HAB_CST_HSM=1)"
    echo "    TDX_IMX_HAB_CST_SRK_INDEX Index of the SRK to be used for signing (1..4)"
    echo "                              Only used when HSM is enabled (TDX_IMX_HAB_CST_HSM=1)"
    echo "    TDX_IMX_HAB_CST_TEMPLATE  Name of the Command Sequence File (CSF) file"
    echo "    UNSIGNED_IMAGE            Path to unsigned image"
    echo "    LOG_MKIMAGE               Path to mkimage log file"
    echo
    echo " Optional Environment Variables:"
    echo
    echo "    TDX_IMX_HAB_CST_ARGS      Additional parameters to be passed to the CST tool"
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
    if [ -z "${TARGET}" ]; then
        error "target argument required"
    fi
    check_fileref "${TDX_IMX_HAB_CST_BIN}" "TDX_IMX_HAB_CST_BIN"
    check_fileref "${TDX_IMX_HAB_CST_SRK}" "TDX_IMX_HAB_CST_SRK"

    # Only check certificates if HSM is disabled (when HSM is used, these
    # variables are actually PKCS11 URIs)
    if [ "${TDX_IMX_HAB_CST_HSM}" = 0 ]; then
        check_fileref "${TDX_IMX_HAB_CST_SRK_CERT}" "TDX_IMX_HAB_CST_SRK_CERT"
        if [ "${TDX_IMX_HAB_CST_SRK_CERT##*_ca_}" = "crt.pem" ]; then
            check_fileref "${TDX_IMX_HAB_CST_SGK_CERT}" "TDX_IMX_HAB_CST_SGK_CERT"
        fi
    fi

    check_fileref "${TDX_IMX_HAB_CST_TEMPLATE}" "TDX_IMX_HAB_CST_TEMPLATE"
    check_fileref "${UNSIGNED_IMAGE}" "UNSIGNED_IMAGE"
    check_fileref "${LOG_MKIMAGE}" "LOG_MKIMAGE"

    # Add "-b pkcs11" if HSM is enabled and it's not already present
    if [ "${TDX_IMX_HAB_CST_HSM}" = 1 ]; then
        case " ${TDX_IMX_HAB_CST_ARGS} " in
            *" -b pkcs11 "*) ;;
            *) TDX_IMX_HAB_CST_ARGS="${TDX_IMX_HAB_CST_ARGS} -b pkcs11" ;;
        esac
    fi
}

# Generate a CSF text file (to be used as input to the CST tool) based on a template.
generate_csf_ahab() {
    local image_csf="${DIR_SCRIPT}/${TARGET}.csf"

    # Copy template file:
    echo "Creating CSF file: ${image_csf}"
    cp "${DIR_SCRIPT}/${TDX_IMX_HAB_CST_TEMPLATE}" "${image_csf}"

    # Determine key index
    local kidx
    if [ "${TDX_IMX_HAB_CST_HSM}" = 1 ]; then
        kidx=${TDX_IMX_HAB_CST_SRK_INDEX}
    else
        kidx=${TDX_IMX_HAB_CST_SRK_CERT##*/}
        kidx=${kidx##SRK}
        kidx=${kidx%%_*}
    fi

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
    if [ "${TDX_IMX_HAB_CST_HSM}" = 1 ]; then
        ca=${TDX_IMX_HAB_CST_SRK_CA}
    elif [ "${TDX_IMX_HAB_CST_SRK_CERT##*_ca_}" = "crt.pem" ]; then
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
    sed -i "s|@@CST_KEY@@|${TDX_IMX_HAB_CST_SRK_CERT}|g" "${image_csf}"
    sed -i "s|@@CST_KIDX@@|${kidx}|g" "${image_csf}"

    if [ "$ca" = "0" ] || [ "${TDX_IMX_HAB_CST_SGK_SUPP}" = "0" ]; then
        # SGK not supported or CA flag is not set: an SGK is not expected.
        # Remove SGK block from CSF.
        echo "Inferred that CA flag was not set; signing with SRK only."
        sed -i "/#+START_SGK_BLOCK/,/#+END_SGK_BLOCK/d" "${image_csf}"
    else
        # CA flag is set: an SGK is expected.
        local sgk="${TDX_IMX_HAB_CST_SGK_CERT?TDX_IMX_HAB_CST_SGK_CERT must be set}"
        echo "Inferred that CA flag was set; signing with SRK and SGK."
        sed -i "/#+START_SGK_BLOCK/d; /#+END_SGK_BLOCK/d" "${image_csf}"
        sed -i "s|@@CST_SGK@@|${sgk}|g" "${image_csf}"
    fi

    # Get offset from log:
    local header block
    header=$(grep "CST: CONTAINER 0 offset:" "${LOG_MKIMAGE}" | tail -1 | awk '{print $5}')
    block=$(grep "CST: CONTAINER 0: Signature Block" "${LOG_MKIMAGE}" | tail -1 | awk '{print $9}')

    # Update offset:
    sed -i "s|@@FLASH.BIN@@|${UNSIGNED_IMAGE}|g" "${image_csf}"
    sed -i "s|@@OFFSETS_ROW:.*$|Offsets = ${header} ${block}|g" "${image_csf}"

    echo "Signing '${UNSIGNED_IMAGE}' with CST tool."
    echo "Tool location: '${TDX_IMX_HAB_CST_BIN}'"
    echo "CSF location: '${image_csf}'"

    # Sign:
    "${TDX_IMX_HAB_CST_BIN}" ${TDX_IMX_HAB_CST_ARGS} -i "${image_csf}" -o "${UNSIGNED_IMAGE}-signed"
}

parse_args "$@"

# Print command for Yocto logs
echo "$0" "$@"

cd "${DIR_SCRIPT}"

validate_environ
generate_csf_ahab
