# override for conditional assignment
DISTROOVERRIDES .= ":tdx-tezi-data-partition"

# data partition filesystem type
# supported values: ext2, ext3, ext4, fat, ubifs
# obs.: this is limited to what Easy Installer supports
TDX_TEZI_DATA_PARTITION_TYPE ?= "ext4"

# data partition label
TDX_TEZI_DATA_PARTITION_LABEL ?= "DATA"

# automount data partition at boot time; possible values:
#  "-1": do not change fstab
#   "0": add entry to fstab with the noauto option
#   "1": add entry to fstab with the auto option
TDX_TEZI_DATA_PARTITION_AUTOMOUNT ?= "${TDX_TEZI_DATA_PARTITION_AUTOMOUNT_DEFAULT}"

# define the default value of above variable based on tdx-encrypted being
# in use or not; notice TDX_TEZI_DATA_PARTITION_AUTOMOUNT_DEFAULT is for
# internal usage of the present class and it should not be modified by
# users (who should only care about TDX_TEZI_DATA_PARTITION_AUTOMOUNT)
TDX_TEZI_DATA_PARTITION_AUTOMOUNT_DEFAULT = "1"
TDX_TEZI_DATA_PARTITION_AUTOMOUNT_DEFAULT:tdx-encrypted = "-1"

# data partition mount point
TDX_TEZI_DATA_PARTITION_MOUNTPOINT ?= "/data"

# data partition mount flags
TDX_TEZI_DATA_PARTITION_MOUNT_FLAGS ?= "rw,nosuid,nodev,noatime,errors=remount-ro"

# data partition want_maximized setting
# "0": do not maximize partition
# "1": maximize partition size if multiple partitions share this setting
#      distributing remaining space evenly
TDX_TEZI_DATA_PARTITION_WANT_MAXIMIZED ?= "1"

# image_type_tezi.bbclass configuration
TEZI_DATA_ENABLED = "1"
TEZI_DATA_FSTYPE = "${TDX_TEZI_DATA_PARTITION_TYPE}"
TEZI_DATA_LABEL = "${TDX_TEZI_DATA_PARTITION_LABEL}"
TEZI_DATA_WANT_MAXIMIZED = "${TDX_TEZI_DATA_PARTITION_WANT_MAXIMIZED}"

# check if tezi image is enabled
addhandler validate_tezi_support
validate_tezi_support[eventmask] = "bb.event.SanityCheck"
python validate_tezi_support() {
    enabled_images = e.data.getVar('IMAGE_FSTYPES')
    if 'teziimg' not in enabled_images:
        bb.fatal("tdx-tezi-data-partition class only works with Easy Installer images, and teziimg is not enabled!")
}
python validate_tezi_support:k3r5 () {
    pass
}
