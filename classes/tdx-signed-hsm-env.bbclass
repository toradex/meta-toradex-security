DEPENDS:append = "\
    openssl-native \
    libp11-native \
    ${TDX_SIGNED_HSM_PKCS11_MODULE_PROVIDER} \
"

tdx_signed_hsm_vars_export() {
    # OpenSSL engines directory
    for d in \
        "${RECIPE_SYSROOT_NATIVE}${libdir}/engines-3" \
        "${RECIPE_SYSROOT_NATIVE}${libdir}/engines-1.1"
    do
        if [ -d "$d" ]; then
            export OPENSSL_ENGINES="$d"
            break
        fi
    done
    [ -d "${OPENSSL_ENGINES}" ] || bbfatal "OpenSSL engines directory not found in native sysroot."

    # HSM token PIN
    if [ -z "${TDX_SIGNED_HSM_TOKEN_PIN}" ]; then
        bbfatal "Missing PIN to access the HSM. Please configure it via TDX_SIGNED_HSM_TOKEN_PIN variable."
    fi
    export MKIMAGE_SIGN_PIN="${TDX_SIGNED_HSM_TOKEN_PIN}"

    # PKCS#11 module
    export PKCS11_MODULE_PATH="${RECIPE_SYSROOT_NATIVE}/${TDX_SIGNED_HSM_PKCS11_MODULE_PATH}"
    [ -r "${PKCS11_MODULE_PATH}" ] || bbfatal "PKCS#11 module missing or not readable: ${PKCS11_MODULE_PATH}"

    # SoftHSM configuration
    if echo ${TDX_SIGNED_HSM_PKCS11_MODULE_PATH} | grep -q 'libsofthsm2.so'; then
        [ -r "${TDX_SIGNED_HSM_PKCS11_SOFTHSM_CONF}" ] || bbfatal "Cannot access SoftHSM configuration file: ${TDX_SIGNED_HSM_PKCS11_SOFTHSM_CONF}"
        export SOFTHSM2_CONF="${TDX_SIGNED_HSM_PKCS11_SOFTHSM_CONF}"
    fi
}
