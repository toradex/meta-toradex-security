# HSM token PIN
# Example: 123456
TDX_SIGNED_HSM_TOKEN_PIN ?= ""

# native package that provides a PKCS11 implementation for the HSM
# it will be added as a dependency on recipes that need to access the HSM
# Example: "softhsm-native"
TDX_SIGNED_HSM_PKCS11_MODULE_PROVIDER ?= ""

# PKCS11 library installation path (relative to the recipe's native sysroot)
# it will be used to configure the PKCS11 module path for OpenSSL
# Example: "/usr/lib/softhsm/libsofthsm2.so"
TDX_SIGNED_HSM_PKCS11_MODULE_PATH ?= ""

# HSM token URL in PKCS#11 URI format to sign the FIT image
# Example: "model=SoftHSM%20v2;manufacturer=SoftHSM%20project;serial=b70b88ff742c63b9;token=virtual-hsm"
TDX_SIGNED_HSM_FIT_TOKEN_URL ?= ""

# HSM token object label of the private key to sign the FIT image
# Example: "fit-sign-key"
TDX_SIGNED_HSM_FIT_TOKEN_LABEL ?= ""

# FIT image configuration to use the HSM for signing
FIT_GENERATE_KEYS = "0"
UBOOT_SIGN_KEYDIR = "${TDX_SIGNED_HSM_FIT_TOKEN_URL}"
UBOOT_SIGN_KEYNAME = "${TDX_SIGNED_HSM_FIT_TOKEN_LABEL}"
UBOOT_MKIMAGE_SIGN_ARGS:append = " -N pkcs11"

# SoftHSM configuration file
TDX_SIGNED_HSM_PKCS11_SOFTHSM_CONF ?= "${TOPDIR}/keys/softhsm/conf/softhsm2.conf"

# HSM-backed HAB signing requires a CST binary with PKCS#11 support.
# Since the externally provided CST may not include that support, force
# the use of a source-built CST and enable its PKCS#11 integration when
# HAB signing is active.
TDX_IMX_HAB_CST_BUILD_FROM_SOURCE = "${TDX_IMX_HAB_ENABLE}"
TDX_IMX_HAB_CST_BUILD_WITH_PKCS11 = "${TDX_IMX_HAB_CST_BUILD_FROM_SOURCE}"

# suppress warning messages from this class
TDX_SIGNED_HSM_SUPPRESS_WARNINGS ?= "0"

addhandler validate_hsm_configuration
validate_hsm_configuration[eventmask] = "bb.event.SanityCheck"
python validate_hsm_configuration() {
    if e.data.getVar("TDX_SIGNED_HSM_SUPPRESS_WARNINGS") == "0":
        if e.data.getVar("TDX_SIGNED_HSM_FIT_TOKEN_URL") == "":
            bb.warn("Token URL to sign the FIT image is not set. Please configure it via TDX_SIGNED_HSM_FIT_TOKEN_URL variable.")
        if e.data.getVar("TDX_SIGNED_HSM_FIT_TOKEN_LABEL") == "":
            bb.warn("Token label to sign the FIT image is not set. Please configure it via TDX_SIGNED_HSM_FIT_TOKEN_LABEL variable.")
        if e.data.getVar("TDX_SIGNED_HSM_PKCS11_MODULE_PROVIDER") == "":
            bb.warn("Missing PKCS11 module provider. Please configure it via TDX_SIGNED_HSM_PKCS11_MODULE_PROVIDER variable.")
        if e.data.getVar("TDX_SIGNED_HSM_PKCS11_MODULE_PATH") == "":
            bb.warn("Missing PKCS11 module path. Please configure it via TDX_SIGNED_HSM_PKCS11_MODULE_PATH variable.")
}
