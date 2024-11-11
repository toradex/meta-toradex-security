#!/bin/sh

# Toradex encryption handler

# backend used to manage the encryption key
TDX_ENC_KEY_BACKEND="@@TDX_ENC_KEY_BACKEND@@"

# encryption key location
TDX_ENC_KEY_LOCATION="@@TDX_ENC_KEY_LOCATION@@"

# directory to store CAAM encrypted key
TDX_ENC_KEY_DIR="@@TDX_ENC_KEY_DIR@@"

# key file name
TDX_ENC_KEY_FILE="@@TDX_ENC_KEY_FILE@@"

# storage location to be encrypted (e.g. partition)
TDX_ENC_STORAGE_LOCATION="@@TDX_ENC_STORAGE_LOCATION@@"

# Number of blocks to reserve from the partition to be encrypted
# Useful in case one needs a storage location to save data in raw
# mode, outside the dm-drypt partition
TDX_ENC_STORAGE_RESERVE="@@TDX_ENC_STORAGE_RESERVE@@"

# number of blocks in the storage to be encrypted by dm-crypt
# depends on the size of the partition and the number of blocks
# reserved (see TDX_ENC_STORAGE_RESERVE), initialized at runtime
TDX_ENC_STORAGE_NUM_BLOCKS=""

# directory to mount the encrypted storage
TDX_ENC_STORAGE_MOUNTPOINT="@@TDX_ENC_STORAGE_MOUNTPOINT@@"

# dm-crypt device to be created
TDX_ENC_DM_DEVICE="encdata"

# Flag to enable preservation of data on partition before encryption
TDX_ENC_PRESERVE_DATA=@@TDX_ENC_PRESERVE_DATA@@

# storage location of data backup file (if needed)
TDX_ENC_BACKUP_FILE="/tmp/encdata.tar.bz2"

# Configurable RAM use percentage
TDX_ENC_BACKUP_STORAGE_PCT=@@TDX_ENC_BACKUP_STORAGE_PCT@@

# encryption key full path
TDX_ENC_KEY_FULLPATH="${TDX_ENC_KEY_DIR}/${TDX_ENC_KEY_FILE}"

# name of the key in the kernel keyring
TDX_ENC_KEY_KEYRING_NAME="tdxenc"

# type of the key in the kernel keyring (depends on the backend and initialized at runtime)
TDX_ENC_KEY_KEYRING_TYPE=""

# log to standard output
tdx_enc_log() {
    echo "${TDX_ENC_KEY_BACKEND}: $*"
}

# log error message and exit
tdx_enc_exit_error() {
    tdx_enc_log "ERROR: $*"
    exit 1
}

# All backends: prepare and check system
tdx_enc_prepare_generic() {
    tdx_enc_log "Preparing and checking system (generic)..."

    if ! modprobe dm-crypt; then
        tdx_enc_exit_error "Error loading dm-crypt module!"
    fi

    if ! dmsetup targets | grep crypt -q; then
        tdx_enc_exit_error "No support for dm-crypt target!"
    fi

    TDX_ENC_STORAGE_NUM_BLOCKS=$(blockdev --getsz ${TDX_ENC_STORAGE_LOCATION})
    if [ "${TDX_ENC_KEY_LOCATION}" = "partition" ] && [ "${TDX_ENC_STORAGE_RESERVE}" = "0" ]; then
        # we need at least one reserved block to store the encryption key blob
        TDX_ENC_STORAGE_RESERVE="1"
    fi
    TDX_ENC_STORAGE_NUM_BLOCKS=$((TDX_ENC_STORAGE_NUM_BLOCKS - TDX_ENC_STORAGE_RESERVE))

    tdx_enc_log "Blocks to be encrypted: $TDX_ENC_STORAGE_NUM_BLOCKS..."
    tdx_enc_log "Reserved blocks: $TDX_ENC_STORAGE_RESERVE..."

    if [ "${TDX_ENC_KEY_LOCATION}" = "partition" ]; then
        tdx_enc_key_recover_from_partition
    fi
}

# CLEARTEXT: prepare system
tdx_enc_prepare_cleartext() {
    tdx_enc_log "Preparing and checking system (cleartext)..."
}

# CAAM: prepare system
tdx_enc_prepare_caam() {
    tdx_enc_log "Preparing and checking system (caam)..."

    if ! modprobe trusted source=caam; then
        tdx_enc_exit_error "Error loading trusted module!"
    fi

    if ! grep -q cbc-aes-caam /proc/crypto; then
        tdx_enc_exit_error "No support for cbc-aes-caam!"
    fi
}

# TPM: prepare system
tdx_enc_prepare_tpm() {
    tdx_enc_log "Preparing and checking system (tpm)..."

    if ! modprobe trusted source=tpm; then
        tdx_enc_exit_error "Error loading trusted module!"
    fi

    if [ ! -c /dev/tpm0 ]; then
        tdx_enc_exit_error "TPM device node (/dev/tpm0) not found!"
    fi

    if ! echo "deadbeef" | tpm2_hash >/dev/null; then
        tdx_enc_exit_error "Hash calculation via tpm2_hash failed. TPM device might not be functional!"
    fi
}

tdx_enc_key_recover_from_partition() {
    tdx_enc_log "Recovering encrypted key blob from partition ${TDX_ENC_STORAGE_LOCATION}..."

    TDX_ENC_KEY_FULLPATH="/tmp/${TDX_ENC_KEY_FILE}"
    rm -rf $TDX_ENC_KEY_FULLPATH

    STORAGE_KEY_BLOCK_DATA=/tmp/.enckey.txt
    if ! dd if=${TDX_ENC_STORAGE_LOCATION} of=${STORAGE_KEY_BLOCK_DATA} skip=${TDX_ENC_STORAGE_NUM_BLOCKS} bs=512 count=1; then
        tdx_enc_exit_error "Could not read block from ${TDX_ENC_STORAGE_LOCATION} with key information!"
    fi

    EOT=$(printf '\004')
    while read -r line; do
        echo "$line" | grep -q "$EOT" && break
        key=$(echo "$line" | cut -d'=' -f1)
        val=$(echo "$line" | cut -d'=' -f2)
        case "$key" in
            "keyname") keyname="${val}" ;;
            "keydata") keydata="${val}" ;;
            "keycsum") keycsum="${val}" ;;
        esac
    done < ${STORAGE_KEY_BLOCK_DATA}

    if [ "${keyname}" != "${TDX_ENC_KEY_KEYRING_NAME}" ]; then
        tdx_enc_log "Invalid key name! A new key will be created."
        return 1
    fi

    if [ -z "${keydata}" ]; then
        tdx_enc_log "Invalid key data! A new key will be created."
        return 1
    fi

    csum=$(printf "%s" "${keydata}" | sha256sum | cut -d' ' -f1)
    if [ "${csum}" != "${keycsum}" ]; then
        tdx_enc_log "Invalid checksum! A new key will be created."
        return 1
    fi

    echo "${keydata}" > ${TDX_ENC_KEY_FULLPATH}
    tdx_enc_log "Encrypted key blob successfully recovered from partition."
}

tdx_enc_key_save_to_partition() {
    tdx_enc_log "Saving encrypted key to partition ${TDX_ENC_STORAGE_LOCATION}..."

    STORAGE_KEY_BLOCK_DATA="/tmp/.enckey.txt"
    {
        echo "keyname=${TDX_ENC_KEY_KEYRING_NAME}"
        echo "keydata=$(cat ${TDX_ENC_KEY_FULLPATH})"
        echo "keycsum=$(sha256sum ${TDX_ENC_KEY_FULLPATH} | cut -d' ' -f1)"
        printf "\04"
    } > ${STORAGE_KEY_BLOCK_DATA}

    if ! dd if=${STORAGE_KEY_BLOCK_DATA} of=${TDX_ENC_STORAGE_LOCATION} seek=${TDX_ENC_STORAGE_NUM_BLOCKS} bs=512; then
        tdx_enc_exit_error "Could not save encrypted key to partition ${TDX_ENC_STORAGE_LOCATION}!"
    fi
}

# configure key in kernel keyring
tdx_enc_keyring_configure() {
    TDX_ENC_KEY_KEYRING_TYPE="$1"
    KEYNAME="$2"
    NEW_KEY_CMD="$3"
    LOAD_KEY_CMD="$4"

    tdx_enc_log "Configuring key in kernel keyring (type=$TDX_ENC_KEY_KEYRING_TYPE keyname=$KEYNAME)..."

    if [ ! -e "${TDX_ENC_KEY_FULLPATH}" ]; then
        tdx_enc_log "Key blob not found. Creating it..."
        KEYHANDLE="$(keyctl add "${TDX_ENC_KEY_KEYRING_TYPE}" "${KEYNAME}" "$(eval echo ${NEW_KEY_CMD})" @s)"
        mkdir -p "${TDX_ENC_KEY_DIR}"
        if ! keyctl pipe "$KEYHANDLE" > "${TDX_ENC_KEY_FULLPATH}"; then
            tdx_enc_exit_error "Error saving key blob!"
        fi
        if [ "${TDX_ENC_KEY_LOCATION}" = "partition" ]; then
            tdx_enc_key_save_to_partition
        fi
    else
        tdx_enc_log "Encrypted key exists. Importing it..."
        keyctl add "${TDX_ENC_KEY_KEYRING_TYPE}" "${KEYNAME}" "$(eval echo ${LOAD_KEY_CMD})" @s
    fi

    if ! keyctl list @s | grep -q "${TDX_ENC_KEY_KEYRING_TYPE}: ${KEYNAME}"; then
        tdx_enc_exit_error "Error adding key to kernel keyring!"
    fi
}

# CLEARTEXT: generate/load key
# the key is generated by using the SoM serial number and no salt,
# so it is reproducible and doesn't need to be stored in a
# persistent storage device. This is very insecure, but we
# don't care about it, since the 'cleartext' backend is only
# for testing purposes.
tdx_enc_key_gen_cleartext() {
    tdx_enc_log "Setting up encryption key for cleartext backend..."
    SN=$(cat /sys/firmware/devicetree/base/serial-number)
    KEY=$(openssl enc -pbkdf2 -aes-128-ecb -nosalt -k "${SN}" -P | cut -d'=' -f 2)
    tdx_enc_keyring_configure "user" "${TDX_ENC_KEY_KEYRING_NAME}" "${KEY}" "${KEY}"
}

# CAAM: generate/load key
tdx_enc_key_gen_caam() {
    tdx_enc_log "Setting up encryption key for CAAM backend..."
    tdx_enc_keyring_configure "trusted" "${TDX_ENC_KEY_KEYRING_NAME}" "new 32" "load \$(cat ${TDX_ENC_KEY_FULLPATH})"
}

# TPM: generate/load key
tdx_enc_key_gen_tpm() {
    tdx_enc_log "Setting up encryption key for TPM backend..."

    if [ ! -e "${TDX_ENC_KEY_FULLPATH}" ]; then

        # create a private RSA key in the TPM
        if ! tpm2_createprimary -C o -G rsa2048 -c /tmp/key.ctxt; then
            tdx_enc_exit_error "Error creating a private RSA key in the TPM!"
        fi

        # make the key persistent
        TPMKEYHANDLE=$(tpm2_evictcontrol -C o -c /tmp/key.ctxt | grep persistent-handle | cut -d' ' -f 2)
        if [ -z "$TPMKEYHANDLE" ]; then
            tdx_enc_exit_error "Error making the TPM key persistent!"
        fi
    fi

    tdx_enc_keyring_configure "trusted" "${TDX_ENC_KEY_KEYRING_NAME}" \
                              "new 32 keyhandle=$TPMKEYHANDLE" \
                              "load \$(cat ${TDX_ENC_KEY_FULLPATH})"
}

# backup original data in partition (if not encrypted)
tdx_enc_backup_data() {
    if [ ${TDX_ENC_PRESERVE_DATA} -ne 1 ]; then
        tdx_enc_log "Data preservation is not enabled"
        return 0
    fi

    mkdir -p "${TDX_ENC_STORAGE_MOUNTPOINT}"
    if ! mount ${TDX_ENC_STORAGE_LOCATION} "${TDX_ENC_STORAGE_MOUNTPOINT}"; then
        return 0
    fi

    tdx_enc_log "Backing up original content..."
    TDX_ENC_BACKUP_FILE=$(mktemp)
    MEM_FREE=$(grep MemFree: /proc/meminfo | tr -s ' ' | cut -d ' ' -f 2)
    BACKUP_STORAGE_LIMIT=$((MEM_FREE * TDX_ENC_BACKUP_STORAGE_PCT / 100))
    tdx_enc_log "Backup limit determined: ${BACKUP_STORAGE_LIMIT}"
    
    msgs="$({ { tar -C "${TDX_ENC_STORAGE_MOUNTPOINT}" -c . || echo "ERROR" >&2; } | { bzip2 -cz || echo "ERROR" >&2; } | dd bs=1024 count=${BACKUP_STORAGE_LIMIT} of=${TDX_ENC_BACKUP_FILE}; } 2>&1)"
    if [ "$?" -ne 0 ] || echo "${msgs}" | grep -qi 'error\|invalid'; then
        tdx_enc_exit_error "Couldn't save original data."
    fi

    tdx_enc_log "Backup created at: ${TDX_ENC_BACKUP_FILE}"
    umount "${TDX_ENC_STORAGE_MOUNTPOINT}"
}

# setup partition with dm-crypt
tdx_enc_partition_setup() {
    tdx_enc_log "Setting up partition with dm-crypt..."

    if ! dmsetup -v create ${TDX_ENC_DM_DEVICE} \
                 --table "0 ${TDX_ENC_STORAGE_NUM_BLOCKS} \
                 crypt capi:cbc(aes)-plain :32:${TDX_ENC_KEY_KEYRING_TYPE}:${TDX_ENC_KEY_KEYRING_NAME} \
                 0 ${TDX_ENC_STORAGE_LOCATION} 0 1 sector_size:512"; then
        tdx_enc_exit_error "Error setting up dm-crypt partition!"
    fi

    if ! dmsetup table --showkey ${TDX_ENC_DM_DEVICE} | grep -q ${TDX_ENC_KEY_KEYRING_NAME}; then
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
    if [ ${TDX_ENC_PRESERVE_DATA} -ne 1 ]; then
        tdx_enc_log "Data preservation is not enabled"
        return 0
    fi

    if ! [ -f ${TDX_ENC_BACKUP_FILE} ]; then
        tdx_enc_log "No data backup to restore"
        return 0
    fi

    tdx_enc_log "Restoring original content..."
    msgs="$({ { bzip2 -cd ${TDX_ENC_BACKUP_FILE} || echo "ERROR" >&2; } | tar -C ${TDX_ENC_STORAGE_MOUNTPOINT} -xf -; } 2>&1)"
    if [ "$?" -ne 0 ] || echo "${msgs}" | grep -qi 'error\|invalid'; then
        tdx_enc_exit_error "Failed to restore backup."
    fi

    rm -rf ${TDX_ENC_BACKUP_FILE}
}

# remove key from keyring
tdx_enc_clear_keys_keyring() {
    tdx_enc_log "Removing key from kernel keyring..."
    keyctl clear @s
}

# umount partition
tdx_enc_partition_umount() {
    for mnt in $(lsblk /dev/mapper/"${TDX_ENC_DM_DEVICE}" -n -o MOUNTPOINTS); do
        tdx_enc_log "Unmounting dm-crypt partition from '${mnt}'..."
        umount "${mnt}"
    done
}

# remove dm-crypt partition
tdx_enc_partition_remove() {
    tdx_enc_log "Removing dm-crypt partition..."
    dmsetup remove ${TDX_ENC_DM_DEVICE}
}

# mount encrypted partition
tdx_enc_main_start() {
    tdx_enc_prepare_generic
    tdx_enc_prepare_${TDX_ENC_KEY_BACKEND}
    tdx_enc_key_gen_${TDX_ENC_KEY_BACKEND}
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
