# override for conditional assignment
DISTROOVERRIDES:append = ":tdx-tezi-data-partition"

# data partition filesystem type
# supported values: ext2, ext3, ext4, fat, ubifs
# obs.: this is limited to what Easy Installer supports
TDX_TEZI_DATA_PARTITION_TYPE ?= "ext4"

# data partition label
TDX_TEZI_DATA_PARTITION_LABEL ?= "DATA"

# automount data partition at boot time
TDX_TEZI_DATA_PARTITION_AUTOMOUNT ?= "1"

# data partition mount point
TDX_TEZI_DATA_PARTITION_MOUNTPOINT ?= "/data"

# data partition mount flags
TDX_TEZI_DATA_PARTITION_MOUNT_FLAGS ?= "rw,nosuid,nodev,noatime,errors=remount-ro"

# image_type_tezi.bbclass configuration
TEZI_DATA_ENABLED = "1"
TEZI_DATA_FSTYPE = "${TDX_TEZI_DATA_PARTITION_TYPE}"
TEZI_DATA_LABEL = "${TDX_TEZI_DATA_PARTITION_LABEL}"

# check if tezi image is enabled
addhandler validate_tezi_support
validate_tezi_support[eventmask] = "bb.event.SanityCheck"
python validate_tezi_support() {
    enabled_images = e.data.getVar('IMAGE_FSTYPES')
    if 'teziimg' not in enabled_images:
        bb.fatal("tdx-tezi-data-partition class only works with Easy Installer images, and teziimg is not enabled!")
}
