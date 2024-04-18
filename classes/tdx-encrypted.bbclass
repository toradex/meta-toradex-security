# Enable encryption support
TDX_ENC_ENABLE = "1"

# Encryption key backend
# This variable defines how the encryption key is managed
# Available options:
#    cleartext -> key is stored in clear text (for testing purposes only!)
#    caam      -> use CAAM (available only on iMX based SoMs)
TDX_ENC_KEY_BACKEND ?= ""
TDX_ENC_KEY_BACKEND:imx-generic-bsp ?= "caam"

# Location in the filesystem to store the encryption key
# Required only for the 'cleartext' backend
TDX_ENC_KEY_FILE ?= "/run/.tdx-enc-key"

# Directory to store CAAM keys and blobs (only for the CAAM backend)
TDX_ENC_CAAM_KEYBLOB_DIR ?= "/var/local/private/caam/keys/"

# Type of encryption
# This variable defines what will be encrypted
# Available options:
#    partition -> encrypt a full partition
TDX_ENC_STORAGE_TYPE ?= "partition"

# Location of the storage to be encrypted
TDX_ENC_STORAGE_LOCATION ?= "/dev/sda1"

# Defines where the encrypted storage will be mounted
TDX_ENC_STORAGE_MOUNTPOINT ?= "/run/encdata"

# tdx-enc-handler provides the scripts to handle encryption
IMAGE_INSTALL:append = " tdx-enc-handler"

# validate encryption parameters
addhandler validate_enc_parameters
validate_enc_parameters[eventmask] = "bb.event.SanityCheck"
python validate_enc_parameters() {
    key_backend = e.data.getVar('TDX_ENC_KEY_BACKEND')
    if key_backend == "":
        bb.fatal("Please set key backend provider via TDX_ENC_KEY_BACKEND.")
    supported_key_backends = ['cleartext','caam']
    if key_backend not in supported_key_backends:
        bb.fatal("'%s' is invalid. Please set a valid key backend provider via TDX_ENC_KEY_BACKEND." % key_backend)
}
