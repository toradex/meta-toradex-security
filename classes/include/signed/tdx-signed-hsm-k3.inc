# PKCS#11 URI of the private key used by binman to sign the TI K3 boot container
# Example: "pkcs11:token=k3-hsm;object=custMpk;type=private"
TDX_SIGNED_HSM_K3_BINMAN_KEY_URL ?= ""

# Path to an OpenSSL configuration file activating the pkcs11 provider.
# When empty, one is generated automatically at build time using the
# PKCS#11 module path declared in TDX_SIGNED_HSM_PKCS11_MODULE_PATH.
TDX_SIGNED_HSM_K3_OPENSSL_CONF ?= ""

addhandler validate_hsm_configuration_k3
validate_hsm_configuration_k3[eventmask] = "bb.event.SanityCheck"
python validate_hsm_configuration_k3() {
    if e.data.getVar("TDX_SIGNED_HSM_K3_BINMAN_KEY_URL") == "":
        bb.fatal("PKCS#11 URI to sign the TI K3 boot container is not set. " \
                "Please configure it via TDX_SIGNED_HSM_K3_BINMAN_KEY_URL variable.")
}
