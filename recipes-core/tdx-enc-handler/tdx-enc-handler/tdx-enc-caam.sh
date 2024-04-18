#!/bin/sh

# Toradex encryption handler for 'caam' key storage backend

# directory to store CAAM keys and blobs
TDX_ENC_KEYBLOB_DIR="@@TDX_ENC_CAAM_KEYBLOB_DIR@@"

# storage location to be encrypted (e.g. partition)
TDX_ENC_STORAGE_LOCATION="@@TDX_ENC_STORAGE_LOCATION@@"

# directory to mount the encrypted storage
TDX_ENC_STORAGE_MOUNTPOINT="@@TDX_ENC_STORAGE_MOUNTPOINT@@"

# dm-crypt device to be created
TDX_ENC_DM_DEVICE="encdata"

# CAAM black key name
TDX_ENC_KEY_NAME="dek"

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
    if ! grep -B1 -A2 tk /proc/crypto | grep -q tk-cbc-aes-caam; then
        tdx_enc_exit_error "No support for tk-cbc-aes-caam!"
    fi
}

# check if the black key exists and create one if needed
tdx_enc_key_gen() {
    tdx_enc_log "Checking for the black key..."

    if [ ! -e ${TDX_ENC_KEYBLOB_DIR}/${TDX_ENC_KEY_NAME} ]; then
        tdx_enc_log "Black key not found. Creating it..."
        KEY=${TDX_ENC_KEY_NAME}
        caam-keygen create ${KEY} ccm -s 32
    else
        tdx_enc_log "Black key exists. Importing it..."
        KEY=i${TDX_ENC_KEY_NAME}
        caam-keygen import ${TDX_ENC_KEYBLOB_DIR}/${TDX_ENC_KEY_NAME}.bb ${KEY}
    fi

    if ! keyctl list @s | grep -q tdxenc; then
        tdx_enc_log "Adding key to kernel keyring..."
        if ! cat ${TDX_ENC_KEYBLOB_DIR}/${KEY} | keyctl padd logon tdxenc: @s; then
            tdx_enc_exit_error "Error adding key to kernel keyring!"
        fi
    else
        tdx_enc_log "Key already in the kernel keyring."
    fi
}

# setup partition with dm-crypt
tdx_enc_partition_setup() {
    tdx_enc_log "Setting up partition with dm-crypt..."

    if ! dmsetup -v create ${TDX_ENC_DM_DEVICE} \
                 --table "0 $(blockdev --getsz ${TDX_ENC_STORAGE_LOCATION}) \
                 crypt capi:tk(cbc(aes))-plain :64:logon:tdxenc: \
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

# umount partition
tdx_enc_partition_umount() {
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
    tdx_enc_partition_setup
    tdx_enc_partition_mount
}

# umount encrypted partition
tdx_enc_main_stop() {
    tdx_enc_partition_umount
    tdx_enc_partition_remove
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
