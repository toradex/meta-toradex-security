#!/bin/sh

# Toradex encryption handler for 'cleartext' key storage backend
# The key is generated and stored in clear text in the filesystem.
# IMPORTANT: USE THIS ONLY FOR TESTING PURPOSES!

# directory to store the encryption key
TDX_ENC_KEY_DIR="@@TDX_ENC_KEY_DIR@@"

# key file name
TDX_ENC_KEY_FILE="@@TDX_ENC_KEY_FILE@@"

# storage location to be encrypted (e.g. partition)
TDX_ENC_STORAGE_LOCATION="@@TDX_ENC_STORAGE_LOCATION@@"

# directory to mount the encrypted storage
TDX_ENC_STORAGE_MOUNTPOINT="@@TDX_ENC_STORAGE_MOUNTPOINT@@"

# dm-crypt device to be created
TDX_ENC_DM_DEVICE="encdata"

# log to standard output
tdx_enc_log() {
    echo "cleartext: $*"
}

# log error message and exit
tdx_enc_exit_error() {
    tdx_enc_log "ERROR: $*"
    exit 1
}

# Generate a key by using the SoM serial number and no salt,
# so it is reproducible and doesn't need to be stored in a
# persistent storage device. This is very insecure, but we
# don't care about it, since the 'cleartext' backend is only
# for testing purposes.
tdx_enc_key_gen() {
    if [ ! -e "${TDX_ENC_KEY_DIR}/${TDX_ENC_KEY_FILE}" ]; then
        tdx_enc_log "Generating encryption key..."
        SN=$(cat /sys/firmware/devicetree/base/serial-number)
        KEY=$(openssl enc -pbkdf2 -aes-128-ecb -nosalt -k "${SN}" -P)
        mkdir -p ${TDX_ENC_KEY_DIR}
        echo "${KEY}" | cut -d= -f2 > "${TDX_ENC_KEY_DIR}/${TDX_ENC_KEY_FILE}"
    fi
}

# setup partition with LUKS
tdx_enc_partition_luks_open() {
    tdx_enc_log "Setting up partition with LUKS..."

    # format LUKS partition (if needed)
    if ! cryptsetup -q luksDump "${TDX_ENC_STORAGE_LOCATION}"; then
        tdx_enc_log "Formatting partition with LUKS..."
        cryptsetup --key-file="${TDX_ENC_KEY_DIR}/${TDX_ENC_KEY_FILE}" \
                   --batch-mode \
                   luksFormat "${TDX_ENC_STORAGE_LOCATION}"
    fi

    # open LUKS partition
    if ! cryptsetup --key-file="${TDX_ENC_KEY_DIR}/${TDX_ENC_KEY_FILE}" \
                    --batch-mode \
                    open \
                    "${TDX_ENC_STORAGE_LOCATION}" \
                    "${TDX_ENC_DM_DEVICE}"; then
        tdx_enc_exit_error "Could not open LUKS partition!"
    fi
}

# mount encrypted partition
tdx_enc_partition_mount() {
    tdx_enc_log "Mounting encrypted partition..."

    # format encrypted partition (if not formatted)
    if ! blkid /dev/mapper/"${TDX_ENC_DM_DEVICE}"; then
        tdx_enc_log "Formatting encrypted partition with ext4..."
        mkfs.ext4 -q /dev/mapper/"${TDX_ENC_DM_DEVICE}"
    fi

    # mount encrypted partition
    mkdir -p "${TDX_ENC_STORAGE_MOUNTPOINT}"
    if ! mount -t ext4 /dev/mapper/"${TDX_ENC_DM_DEVICE}" "${TDX_ENC_STORAGE_MOUNTPOINT}"; then
        tdx_enc_exit_error "Could not mount encrypted partition!"
    fi
}

# umount partition
tdx_enc_partition_umount() {
    umount "${TDX_ENC_STORAGE_MOUNTPOINT}"
}

# close LUKS partition
tdx_enc_partition_luks_close() {
    cryptsetup close "${TDX_ENC_DM_DEVICE}"
}

# mount encrypted partition
tdx_enc_main_start() {
    tdx_enc_key_gen
    tdx_enc_partition_luks_open
    tdx_enc_partition_mount
}

# umount encrypted partition
tdx_enc_main_stop() {
    tdx_enc_partition_umount
    tdx_enc_partition_luks_close
}

tdx_enc_main() {
    case $1 in
        start)
            tdx_enc_main_start
            ;;
        stop)
            tdx_enc_main_stop
            ;;
        *)
            tdx_enc_exit_error "Invalid option!"
            ;;
    esac

    tdx_enc_log "Success!"
}

tdx_enc_main "$1"
