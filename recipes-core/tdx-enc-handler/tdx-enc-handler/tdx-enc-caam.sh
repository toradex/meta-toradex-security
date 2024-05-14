#!/bin/sh

# Toradex encryption handler for 'caam' key storage backend

# directory to store CAAM encrypted key
TDX_ENC_KEYBLOB_DIR="@@TDX_ENC_CAAM_KEYBLOB_DIR@@"

# storage location to be encrypted (e.g. partition)
TDX_ENC_STORAGE_LOCATION="@@TDX_ENC_STORAGE_LOCATION@@"

# directory to mount the encrypted storage
TDX_ENC_STORAGE_MOUNTPOINT="@@TDX_ENC_STORAGE_MOUNTPOINT@@"

# dm-crypt device to be created
TDX_ENC_DM_DEVICE="encdata"

# CAAM encrypted key name
TDX_ENC_KEY_NAME="dek.bb"

# log to standard output
tdx_enc_log() {
        echo "CAAM: $*"
}

# log error message and exit
tdx_enc_exit_error() {
    tdx_enc_log "ERROR: $*"
    exit 1
}

# system checks
tdx_enc_check() {
    if ! dmsetup targets | grep crypt -q; then
        tdx_enc_exit_error "No support for dm-crypt target!"
    fi
    if ! grep -q cbc-aes-caam /proc/crypto; then
        tdx_enc_exit_error "No support for cbc-aes-caam!"
    fi
}

# check if the encrypted key exists and create one if needed
tdx_enc_key_gen() {
    tdx_enc_log "Checking for the encrypted key..."

    ENC_KEY_FILE="${TDX_ENC_KEYBLOB_DIR}/${TDX_ENC_KEY_NAME}"

    if [ ! -e "${ENC_KEY_FILE}" ]; then
        tdx_enc_log "Encrypted key not found. Creating it..."
        KEY="$(keyctl add trusted tdxenc 'new 32' @s)"
        mkdir -p "${TDX_ENC_KEYBLOB_DIR}"
        if ! keyctl pipe "$KEY" > "${ENC_KEY_FILE}"; then
            tdx_enc_exit_error "Error saving encrypted key!"
        fi
    else
        tdx_enc_log "Encrypted key exists. Importing it..."
        keyctl add trusted tdxenc "load $(cat ${ENC_KEY_FILE})" @s
    fi

    if ! keyctl list @s | grep -q "trusted: tdxenc"; then
        tdx_enc_exit_error "Error adding key to kernel keyring!"
    fi
}

# backup original data in partition (if not encrypted)
tdx_enc_backup_data() {
    mkdir -p "${TDX_ENC_STORAGE_MOUNTPOINT}"
    if mount -t ext4 ${TDX_ENC_STORAGE_LOCATION} "${TDX_ENC_STORAGE_MOUNTPOINT}"; then
        tdx_enc_log "Backing up original content..."
        
        mkdir /tmp/${TDX_ENC_DM_DEVICE}
        chmod 777 /tmp/${TDX_ENC_DM_DEVICE}
        mount -t tmpfs -o size=$(blockdev --getsz ${TDX_ENC_STORAGE_LOCATION}) ${TDX_ENC_DM_DEVICE} /tmp/${TDX_ENC_DM_DEVICE}
        cp -a ${TDX_ENC_STORAGE_MOUNTPOINT}/. /tmp/${TDX_ENC_DM_DEVICE}/
        umount "${TDX_ENC_STORAGE_MOUNTPOINT}"
    fi
}

# setup partition with dm-crypt
tdx_enc_partition_setup() {
    tdx_enc_log "Setting up partition with dm-crypt..."

    if ! dmsetup -v create ${TDX_ENC_DM_DEVICE} \
                 --table "0 $(blockdev --getsz ${TDX_ENC_STORAGE_LOCATION}) \
                 crypt capi:cbc(aes)-plain :32:trusted:tdxenc \
                 0 ${TDX_ENC_STORAGE_LOCATION} 0 1 sector_size:512"; then
        tdx_enc_exit_error "Error setting up dm-crypt partition!"
    fi

    if ! dmsetup table --showkey encdata | grep -q tdxenc; then
        tdx_enc_exit_error "Key not found in dm-crypt partition!"
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

# restore data if available
tdx_enc_restore_data() {
    if [ -d /tmp/${TDX_ENC_DM_DEVICE} ]; then
        tdx_enc_log "Restoring original content..."
        cp -a /tmp/${TDX_ENC_DM_DEVICE}/. ${TDX_ENC_STORAGE_MOUNTPOINT}/
        umount /tmp/${TDX_ENC_DM_DEVICE}/
        rm -rf /tmp/${TDX_ENC_DM_DEVICE}
    fi
}

# umount partition
tdx_enc_clear_keys_keyring() {
    tdx_enc_log "Removing key from kernel keyring..."
    keyctl clear @s
}

# umount partition
tdx_enc_partition_umount() {
    tdx_enc_log "Unmounting dm-crypt partition..."
    umount "${TDX_ENC_STORAGE_MOUNTPOINT}"
}

# remove dm-crypt partition
tdx_enc_partition_remove() {
    tdx_enc_log "Removing dm-crypt partition..."
    dmsetup remove ${TDX_ENC_DM_DEVICE}
}

# mount encrypted partition
tdx_enc_main_start() {
    tdx_enc_check
    tdx_enc_key_gen
    tdx_enc_backup_data
    tdx_enc_partition_setup
    tdx_enc_partition_mount
    tdx_enc_restore_data
}

# umount encrypted partition
tdx_enc_main_stop() {
    tdx_enc_partition_umount
    tdx_enc_partition_remove
    tdx_enc_clear_keys_keyring
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
            tdx_enc_exit_error "Invalid option! Please use 'start' or 'stop'."
            ;;
    esac

    tdx_enc_log "Success!"
}

tdx_enc_main "$1"
