# Enable encryption support
TDX_ENC_ENABLE = "1"

# Encryption key backend
# This variable defines how the encryption key is managed
# Available options:
#    cleartext -> key is stored in clear text (for testing purposes only!)
#    caam      -> use CAAM (available only on iMX based SoMs)
#    tpm       -> use TPM (Trusted Platform Module)
TDX_ENC_KEY_BACKEND ?= ""
TDX_ENC_KEY_BACKEND:imx-generic-bsp ?= "caam"

# directory to store the encryption key blob
TDX_ENC_KEY_DIR ?= "/var/local/private/.keys"

# encryption key blob file name
TDX_ENC_KEY_FILE ?= "tdx-enc-key.blob"

# Type of encryption
# This variable defines what will be encrypted
# Available options:
#    partition -> encrypt a full partition
TDX_ENC_STORAGE_TYPE ?= "partition"

# Partition to be encrypted (e.g. /dev/sda1)
TDX_ENC_STORAGE_LOCATION ?= ""

# Defines where the encrypted storage will be mounted
TDX_ENC_STORAGE_MOUNTPOINT ?= "/run/encdata"

# Enables preservation of existing data on encrypted device
TDX_ENC_PRESERVE_DATA ?= "0"

# Configurable backup RAM use percentage
TDX_ENC_BACKUP_STORAGE_PCT ?= "30"

# tdx-enc-handler provides the scripts to handle encryption
IMAGE_INSTALL:append = " tdx-enc-handler"

# validate encryption parameters
addhandler validate_enc_parameters
validate_enc_parameters[eventmask] = "bb.event.SanityCheck"
python validate_enc_parameters() {
    key_backend = e.data.getVar('TDX_ENC_KEY_BACKEND')
    if key_backend == "":
        bb.fatal("Please set key backend provider via TDX_ENC_KEY_BACKEND.")
    supported_key_backends = ['cleartext','caam','tpm']
    if key_backend not in supported_key_backends:
        bb.fatal("'%s' is invalid. Please set a valid key backend provider via TDX_ENC_KEY_BACKEND." % key_backend)

    storage_location = e.data.getVar('TDX_ENC_STORAGE_LOCATION')
    if storage_location == "":
        bb.fatal("Please set storage to be encrypted via TDX_ENC_STORAGE_LOCATION.")
}
