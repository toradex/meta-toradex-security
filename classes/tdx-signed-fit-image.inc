# enable FIT image
KERNEL_CLASSES:append = " kernel-fitimage"
KERNEL_IMAGETYPE:forcevariable = "fitImage"

# enable signature checking of FIT images
UBOOT_SIGN_ENABLE ?= "1"
UBOOT_SIGN_ENABLE:verdin-am62-k3r5 = "0"
UBOOT_MKIMAGE_DTCOPTS = "-I dts -O dtb -p 2000"
UBOOT_SIGN_KEYDIR ?= "${TOPDIR}/keys/fit"
UBOOT_SIGN_KEYNAME ?= "dev"
UBOOT_SIGN_IMG_KEYNAME ?= "dev2"

# parameters to generate the keys to sign the FIT image
FIT_GENERATE_KEYS ?= "1"
FIT_KEY_GENRSA_ARGS ?= "-F4"
FIT_KEY_REQ_ARGS ?= "-batch -new"
FIT_KEY_SIGN_PKCS ?= "-x509"

# parameters to sign FIT images
FIT_HASH_ALG ?= "sha256"
FIT_SIGN_ALG ?= "rsa2048"
FIT_SIGN_NUMBITS ?= "2048"
FIT_SIGN_INDIVIDUAL = "0"
