# enable FIT image
KERNEL_CLASSES:append = " kernel-fitimage"
KERNEL_IMAGETYPE:forcevariable = "fitImage"

# enable signature checking of FIT images
UBOOT_SIGN_ENABLE ?= "1"
UBOOT_SIGN_ENABLE:k3r5 = "0"
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

# stronger assignments to override settings in meta-ti
#
# NOTICE: a side effect of these assignments is that users will need to use
#         :forcevariable (or an override stronger than :k3) to set them.
UBOOT_SIGN_KEYDIR:k3 ?= "${TOPDIR}/keys/fit"
UBOOT_SIGN_KEYNAME:k3 ?= "dev"
UBOOT_SIGN_IMG_KEYNAME:k3 ?= "dev2"
FIT_HASH_ALG:k3 ?= "sha256"
FIT_SIGN_ALG:k3 ?= "rsa2048"
FIT_SIGN_NUMBITS:k3 ?= "2048"

# When FIT signing is enabled with filesystem-based keys, fail early if the
# key files are missing. Upstream mkimage prints an error but still exits 0
# when the key cannot be opened, so without this check the build silently
# produces an unsigned FIT image.
addhandler validate_fit_signing_keys
validate_fit_signing_keys[eventmask] = "bb.event.SanityCheck"
python validate_fit_signing_keys() {
    if e.data.getVar("UBOOT_SIGN_ENABLE") != "1":
        return

    # In HSM mode, UBOOT_SIGN_KEYDIR holds a PKCS#11 URI, not a filesystem path
    if e.data.getVar("TDX_SIGNED_HSM") == "1":
        return

    # When auto-generation is enabled, the keys are produced at build time
    if e.data.getVar("FIT_GENERATE_KEYS") == "1":
        return

    keydir = e.data.getVar("UBOOT_SIGN_KEYDIR")
    keyname = e.data.getVar("UBOOT_SIGN_KEYNAME")

    for ext in (".key", ".crt"):
        path = os.path.join(keydir, keyname + ext)
        if not os.path.exists(path):
            bb.fatal("FIT image signing is enabled but signing key file [%s] is missing!" % path)
}
