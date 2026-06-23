FILESEXTRAPATHS:prepend := "${THISDIR}/files:"

# Skip the SM-to-ELE DVFS notifications (ELE_START/STOP_DVFS_CHANGE) that the
# shipped ELE firmware (2.0.2) does not implement. ELE rejects them with
# ELE_INVALID_MESSAGE, logging 0xc0f429/0xc1f429 every boot, which makes
# ahab_close (forward_lifecycle -> OEM closed) fail with
# ELE_AUTH_SKIPPED_OR_FAILED (0xbb29). Drop once a DVFS-capable ELE FW is used.
SRC_URI:append:mx95-generic-bsp = "\
    file://0001-ele-skip-DVFS-notification.patch \
"
