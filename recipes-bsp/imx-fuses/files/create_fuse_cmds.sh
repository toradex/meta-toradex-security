#!/bin/bash

[ "${XTRACE}" = "1" ] && set -x

set -e

# positional parameters
SOC="$1"
FUSE_CMDS_FILE="$2"
TEMPLATES_DIR=${3:-"."}
FUSE_INFO_FILE="$(dirname "$FUSE_CMDS_FILE")/imx-config.fuse"

# environment variables (TDX_*)
SRK_FUSE_FILE="${TDX_IMX_HAB_CST_SRK_FUSE}"

if [ -z "${SRK_FUSE_FILE}" ]; then
    echo "Error: TDX_IMX_HAB_CST_SRK_FUSE must be set in the environment!" >&2
    exit 1
fi

# warning to program SRK hash fuses
WARNING1="\
# These are One-Time Programmable e-fuses. Once you write them you can't
# go back, so get it right the first time!"

# warning to 'close' the device
WARNING2="\
# After the device successfully boots a signed image without generating
# any HAB events, it is safe to secure, or 'close', the device. This is
# the last step in the process. Once the fuse is blown, the chip does
# not load an image that has not been signed using the correct PKI tree.
# Be careful! This is again a One-Time Programmable e-fuse. Once you
# write it you can't go back, so get it right the first time. If
# anything in the previous steps wasn't done correctly, after writing
# this bit, the SOM will not boot anymore!"

# warning header for the secure-debug section
WARNING_SECURE_DEBUG="\
# === Secure Debug fuses ===
# These fuses configure the System JTAG Controller (SJC).
# They are One-Time Programmable e-fuses; the BOOT_CFG_LOCK at
# the end protects the JTAG mode bits against shadow-register override."

fuse_write_line() {
    local bank=$1
    local word=$2
    local hexval=$3
    echo "fuse prog -y $bank $word $hexval"
}

# Emit a fuse-prog line for an SJC template entry, with a leading comment.
# $1 = symbol name (looked up in SJC_TEMPLATE)
# $2 = human-readable comment
# $3 = optional hex value; if omitted, the template's mask is used.
secure_debug_emit() {
    local name="$1"
    local comment="$2"
    local fuseval="${3:-}"

    local entry="${SJC_TEMPLATE[$name]}"
    if [ -z "$entry" ]; then
        echo "Error: SJC template entry '$name' not found!" >&2
        return 1
    fi

    local bank word mask
    read -r bank word mask <<<"$entry"
    local hexval="${fuseval:-$mask}"
    {
        echo "# $comment"
        fuse_write_line "$bank" "$word" "$hexval"
    } >> "$FUSE_CMDS_FILE"
}

secure_debug_load_template() {
    local template_file="$1"
    if [ ! -e "$template_file" ]; then
        echo "Error: SJC template file not found [$template_file]!" >&2
        return 1
    fi

    declare -gA SJC_TEMPLATE
    local prefix name bank word mask
    while IFS=: read -r prefix name bank word mask; do
        [ "$prefix" = "SJC" ] || continue
        SJC_TEMPLATE[$name]="$bank $word $mask"
    done < "$template_file"
}

secure_debug_append() {
    local template_file=""
    case "${SOC}" in
        "IMX8M")
            template_file="${TEMPLATES_DIR}/imx8mm-sjc-template.fuse"
            ;;
        *)
            echo "Error: Secure Debug is not supported for SOC=${SOC}!" >&2
            return 1
            ;;
    esac

    secure_debug_load_template "$template_file"

    echo "" >> "$FUSE_CMDS_FILE"
    echo "${WARNING_SECURE_DEBUG}" >> "$FUSE_CMDS_FILE"

    if [ "${TDX_SECURE_DEBUG_SJC_DISABLE}" = "1" ]; then
        secure_debug_emit SJC_DISABLE "SJC_DISABLE = 1 (full JTAG disable)"
    else
        case "${TDX_SECURE_DEBUG_MODE}" in
            "authenticated")
                local key
                key=$(tr -d '[:space:]' < "${TDX_SECURE_DEBUG_KEY_FILE}")
                if [ ${#key} -ne 14 ] || ! [[ "$key" =~ ^[0-9a-fA-F]+$ ]]; then
                    echo "Error: invalid key file content (must be 14 hex chars)" >&2
                    return 1
                fi
                local key_lo="0x${key:6:8}"
                local key_hi="0x00${key:0:6}"
                secure_debug_emit SJC_RESP_LO "SJC_RESP[31:0]" "$key_lo"
                secure_debug_emit SJC_RESP_HI "SJC_RESP[55:32]" "$key_hi"
                secure_debug_emit SJC_RESP_LOCK "SJC_RESP_LOCK"
                secure_debug_emit JTAG_SMODE_SECURE "JTAG_SMODE = Secure JTAG"
                if [ "${TDX_SECURE_DEBUG_SJC_HEO}" = "1" ]; then
                    secure_debug_emit JTAG_HEO "JTAG_HEO = 1 (block HAB software reopen)"
                fi
                ;;
            "disabled")
                secure_debug_emit JTAG_SMODE_NODEBUG "JTAG_SMODE = No Debug"
                ;;
            *)
                echo "Error: invalid TDX_SECURE_DEBUG_MODE='${TDX_SECURE_DEBUG_MODE}'" >&2
                return 1
                ;;
        esac
        secure_debug_emit KTE "KTE = 1 (gate bus tracing on SJC state)"
    fi

    secure_debug_emit BOOT_CFG_LOCK_OP "BOOT_CFG_LOCK = OP (override-protect JTAG mode)"
}

create_fuse_cmds() {
    local template_file="$1"

    echo "Generating fusing files using template [$template_file]."

    if [ ! -e "$template_file" ]; then
        echo "Template file not found [$template_file]!"
        return 1
    fi

    cp "$template_file" "$FUSE_INFO_FILE"
    echo "${WARNING1}" > "$FUSE_CMDS_FILE"

    echo "Writing fusing commands..."

    for hexval in $(hexdump -e '/4 "0x"' -e '/4 "%X""\n"' "${SRK_FUSE_FILE}"); do
        if ! fuse_info=$(grep -m 1 "^H:F:.*:$" "$FUSE_INFO_FILE"); then
            rm -rf "$FUSE_INFO_FILE" "$FUSE_CMDS_FILE"
            echo "Error: didn't find empty fuse line to insert hex value!"
            return 1
        fi
        bank=$(echo "$fuse_info" | cut -d: -f3)
        word=$(echo "$fuse_info" | cut -d: -f4)
        sed -i "/^$fuse_info/ s/\$/$hexval/" "$FUSE_INFO_FILE"
        fuse_write_line "$bank" "$word" "$hexval" >> "$FUSE_CMDS_FILE"
    done

    if grep -q "^H:F:.*:$" "$FUSE_INFO_FILE"; then
        rm -rf "$FUSE_INFO_FILE" "$FUSE_CMDS_FILE"
        echo "Error: there are still unpopulated fuse values!"
        return 1
    fi

    # Insert the secure-debug section between the SRK hashes and the close
    # command. On AHAB platforms this ordering is required: response-key fuses
    # are only writable while the device is in OEM Open, and the close command
    # transitions it to OEM Closed.
    if [ "${TDX_SECURE_DEBUG_ENABLE}" = "1" ]; then
        echo "Appending Secure Debug fuses..."
        secure_debug_append
    fi

    echo -e "\n${WARNING2}" >> "$FUSE_CMDS_FILE"

    echo "Writing 'close' command..."

    if grep -q "H:T:HAB" "$FUSE_INFO_FILE"; then
        fuse_info=$(grep "^H:C:" "$FUSE_INFO_FILE")
        bank=$(echo "$fuse_info" | cut -d: -f3)
        word=$(echo "$fuse_info" | cut -d: -f4)
        hexval=$(echo "$fuse_info" | cut -d: -f5)
        fuse_write_line "$bank" "$word" "$hexval" >> "$FUSE_CMDS_FILE"
    else
        echo "ahab_close" >> "$FUSE_CMDS_FILE"
    fi

    echo "Fusing files successfully generated!"
}

# Print command for Yocto logs
echo "$0" "$@"

case ${SOC} in
    "IMX6ULL"|"IMX6")
        create_fuse_cmds "${TEMPLATES_DIR}/imx6-template.fuse"
        ;;
    "IMX7")
        create_fuse_cmds "${TEMPLATES_DIR}/imx7-template.fuse"
        ;;
    "IMX8M")
        create_fuse_cmds "${TEMPLATES_DIR}/imx8m-template.fuse"
        ;;
    "iMX8QX")
        create_fuse_cmds "${TEMPLATES_DIR}/imx8qx-template.fuse"
        ;;
    "iMX8QM")
        create_fuse_cmds "${TEMPLATES_DIR}/imx8qm-template.fuse"
        ;;
    "iMX95")
        create_fuse_cmds "${TEMPLATES_DIR}/imx95-template.fuse"
        ;;
    *)
        echo "Invalid SOC!"
        return 1
        ;;
esac
