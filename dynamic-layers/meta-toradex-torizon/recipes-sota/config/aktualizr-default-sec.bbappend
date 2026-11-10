DEPENDS:append:intel-corei7-64:tdx-signed = " jq-native"

do_install:append:intel-corei7-64:tdx-signed() {
    ${STAGING_BINDIR_NATIVE}/jq '."torizon-generic" += [
      {
        "partial_verifying": false,
        "ecu_hardware_id": "uefi-db-append",
        "full_client_dir": "/var/sota/storage/uefi-db-append",
        "ecu_private_key": "sec.private",
        "ecu_public_key": "sec.public",
        "firmware_path": "/var/sota/storage/uefi-db-append/update.auth",
        "target_name_path": "/var/sota/storage/uefi-db-append/target_name",
        "metadata_path": "/var/sota/storage/uefi-db-append/metadata",
        "action_handler_path": "/usr/libexec/uefi-sb-update/apply-uefi-auth-handler.sh"
      },
      {
        "partial_verifying": false,
        "ecu_hardware_id": "uefi-dbx-append",
        "full_client_dir": "/var/sota/storage/uefi-dbx-append",
        "ecu_private_key": "sec.private",
        "ecu_public_key": "sec.public",
        "firmware_path": "/var/sota/storage/uefi-dbx-append/update.auth",
        "target_name_path": "/var/sota/storage/uefi-dbx-append/target_name",
        "metadata_path": "/var/sota/storage/uefi-dbx-append/metadata",
        "action_handler_path": "/usr/libexec/uefi-sb-update/apply-uefi-auth-handler.sh"
      }
    ]' ${D}${libdir}/sota/secondaries.json > ${WORKDIR}/uefi-secondaries.json
    install -m 0644 ${WORKDIR}/uefi-secondaries.json ${D}${libdir}/sota/secondaries.json
}
