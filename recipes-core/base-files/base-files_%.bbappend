do_install:append:tdx-tezi-data-partition() {
    AUTO=$([ "${TDX_TEZI_DATA_PARTITION_AUTOMOUNT}" = "1" ] && echo "auto" || echo "noauto")
    echo "LABEL=DATA  ${TDX_TEZI_DATA_PARTITION_MOUNTPOINT}  auto  ${TDX_TEZI_DATA_PARTITION_MOUNT_FLAGS},${AUTO}  0  0" >> ${D}/etc/fstab
}

pkg_postinst:${PN}:append:tdx-tezi-data-partition() {
    mkdir -p $D${TDX_TEZI_DATA_PARTITION_MOUNTPOINT}
}
