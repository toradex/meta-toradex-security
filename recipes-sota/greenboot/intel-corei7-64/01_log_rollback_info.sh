#!/bin/bash
# Log rollback state after a boot (greenboot green.d).
#
# In green.d (boot succeeded): detect rollback via ostree deployment order.
#   ostree admin status lists deployments newest-first; the booted (*) entry
#   on any position other than first means we are running a rollback deployment.

. /usr/libexec/greenboot/greenboot-logs

is_ostree_rollback() {
    ostree admin status 2>/dev/null | grep -q '\(pending\)'
}

STATUS=$(/usr/lib/systemd/systemd-bless-boot status 2>/dev/null)
case "$STATUS" in
    good|clean)
        if is_ostree_rollback; then
            log_to_console "OTA update failed -- running rollback deployment (previous OTA update rolled back)."
            if [ -e "$LOGFILE" ]; then
                log_to_console "Check rollback log in $LOGFILE for details."
            fi
        else
            # Normal boot, no rollback in effect; remove stale rollback log.
            rm -Rf "$LOGFILE"
        fi
        ;;
    *)
        # Unknown status; clean up stale log.
        rm -Rf "$LOGFILE"
        ;;
esac
