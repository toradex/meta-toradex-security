#!/bin/bash
# Confirm OTA boot success via systemd-boot boot counting.
# Runs in greenboot green.d (boot-complete.target reached).

STATUS=$(/usr/lib/systemd/systemd-bless-boot status 2>/dev/null)
case "$STATUS" in
    indeterminate)
        echo "Confirming boot success via systemd-bless-boot"
        /usr/lib/systemd/systemd-bless-boot good
        ;;
    good|clean)
        echo "Boot already confirmed (status: $STATUS)"
        ;;
    bad|dirty)
        echo "WARNING: boot status is '$STATUS' -- skipping confirmation"
        ;;
    *)
        echo "/usr/lib/systemd/systemd-bless-boot not active or no boot counting in effect (status: '$STATUS')"
        ;;
esac
