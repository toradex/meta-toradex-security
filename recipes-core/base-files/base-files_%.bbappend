do_install:append:tdx-tezi-data-partition() {
    local auto_option=""
    local modify_fstab="0"

    case "${TDX_TEZI_DATA_PARTITION_AUTOMOUNT}" in
        -1)
                modify_fstab="0"
                ;;
        0)
                auto_option="noauto"
                modify_fstab="1"
                ;;
        1)
                auto_option="auto"
                modify_fstab="1"
                ;;
        *)
                bbfatal "Variable TDX_TEZI_DATA_PARTITION_AUTOMOUNT is set to an unknown value (${TDX_TEZI_DATA_PARTITION_AUTOMOUNT})."
                ;;
    esac

    if [ "${modify_fstab}" = "1" ]; then
        echo "LABEL=${TDX_TEZI_DATA_PARTITION_LABEL}  ${TDX_TEZI_DATA_PARTITION_MOUNTPOINT}  auto  ${TDX_TEZI_DATA_PARTITION_MOUNT_FLAGS},${auto_option}  0  0" >> ${D}/etc/fstab
    fi
}

pkg_postinst:${PN}:append:tdx-tezi-data-partition() {
    mkdir -p $D${TDX_TEZI_DATA_PARTITION_MOUNTPOINT}
}
