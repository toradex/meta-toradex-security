#!/bin/bash
# Apply one KEK-signed append update to UEFI db or dbx.

set -e

die() {
    echo "uefi-sb-update: $*" >&2
    exit "$1"
}

var_type="${1:-}"
auth_file="${2:-}"

[[ "$var_type" == db || "$var_type" == dbx ]] || die 1 "invalid variable"
[[ -r "$auth_file" ]] || die 1 "unreadable update payload"
[[ $(id -u) -eq 0 ]] || die 1 "must run as root"

for tool in chattr efi-readvar efi-updatevar; do
    command -v "$tool" >/dev/null || die 1 "missing $tool"
done

efi-readvar -v PK >/dev/null 2>&1 || die 1 "PK absent; firmware is in Setup Mode"
efi-readvar -v KEK >/dev/null 2>&1 || die 1 "KEK absent"

efivar="/sys/firmware/efi/efivars/${var_type}-d719b2cb-3d3a-4596-a3bc-dad00e67656f"
if [[ -e "$efivar" ]]; then
    chattr -i "$efivar" || die 2 "cannot clear immutable attribute"
    restore_immutable() {
        chattr +i "$efivar" || echo "uefi-sb-update: cannot restore immutable attribute" >&2
    }
    trap restore_immutable EXIT
fi

before=$(efi-readvar -v "$var_type" 2>/dev/null || true)
efi-updatevar -a -f "$auth_file" "$var_type" || die 2 "firmware rejected update"

if [[ -e "$efivar" ]]; then
    restore_immutable
    trap - EXIT
fi

after=$(efi-readvar -v "$var_type" 2>/dev/null || true)
[[ "$before" != "$after" ]] || die 3 "UEFI variable unchanged"
