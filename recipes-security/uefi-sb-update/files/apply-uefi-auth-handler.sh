#!/bin/bash
# Aktualizr handler. stdout contains exactly one JSON reply.

set -u

readonly hardware_id="${SECONDARY_HARDWARE_ID:-}"
readonly storage_dir="${SECONDARY_STORAGE_DIR:-/var/sota/storage/${hardware_id}}"
readonly state_file="${storage_dir}/installed.env"
readonly target_name_file="${storage_dir}/target_name"
readonly apply_script="/usr/libexec/uefi-sb-update/apply-uefi-auth.sh"

log() { echo "uefi-sb-update: $*" >&2; }
reply() { printf '{"status":"%s","message":"%s"}\n' "$1" "$2"; }

case "$hardware_id" in
    uefi-db-append) variable=db ;;
    uefi-dbx-append) variable=dbx ;;
    *) reply failed "unsupported UEFI update target"; exit 65 ;;
esac

case "${1:-}" in
    get-firmware-info)
        [[ -r "$state_file" ]] || exit 64
        . "$state_file"
        # The manifest must report the Uptane target name assigned by the
        # Director, otherwise the server never considers the update finished.
        if [[ -r "$target_name_file" ]]; then
            TARGET_NAME=$(cat "$target_name_file")
        fi
        printf '{"name":"%s","sha256":"%s","length":%s,"status":"ok","message":"UEFI update state loaded"}\n' \
            "$TARGET_NAME" "$PAYLOAD_SHA256" "$PAYLOAD_LENGTH"
        ;;
    install)
        payload="${SECONDARY_FIRMWARE_PATH:-}"
        expected_sha256="${SECONDARY_FIRMWARE_SHA256:-}"
        [[ -f "$payload" ]] || { reply failed "UEFI update payload missing"; exit 65; }
        actual_sha256=$(sha256sum "$payload" | awk '{print $1}')
        [[ -z "$expected_sha256" || "$expected_sha256" == "$actual_sha256" ]] || {
            reply failed "UEFI update payload checksum mismatch"; exit 65;
        }
        mkdir -p "$storage_dir"
        "$apply_script" "$variable" "$payload" >&2 || {
            log "${variable} update failed"
            reply failed "UEFI ${variable} update failed"
            exit 65
        }
        cat > "$state_file" <<EOF
TARGET_NAME=${SECONDARY_TARGET_NAME:-${hardware_id}-${actual_sha256}}
PAYLOAD_SHA256=${actual_sha256}
PAYLOAD_LENGTH=$(stat -c%s "$payload")
EOF
        touch /run/need-reboot
        reply need-completion "UEFI ${variable} update applied; reboot pending"
        ;;
    complete-install)
        [[ -r "$state_file" ]] && efi-readvar -v "$variable" >/dev/null 2>&1 || {
            reply failed "UEFI update unavailable after reboot"; exit 65;
        }
        reply ok "UEFI ${variable} update persisted after reboot"
        ;;
    *) reply failed "unsupported UEFI update action"; exit 65 ;;
esac
