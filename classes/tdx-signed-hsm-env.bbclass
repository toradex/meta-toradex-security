DEPENDS:append = "\
    openssl-native \
    libp11-native \
    pkcs11-provider-native \
    ${TDX_SIGNED_HSM_PKCS11_MODULE_PROVIDER} \
"

tdx_signed_hsm_vars_export_common() {
    # HSM token PIN
    if [ -z "${TDX_SIGNED_HSM_TOKEN_PIN}" ]; then
        bbfatal "Missing PIN to access the HSM. Please configure it via TDX_SIGNED_HSM_TOKEN_PIN variable."
    fi

    # SoftHSM configuration
    if echo ${TDX_SIGNED_HSM_PKCS11_MODULE_PATH} | grep -q 'libsofthsm2.so'; then
        [ -r "${TDX_SIGNED_HSM_PKCS11_SOFTHSM_CONF}" ] || bbfatal "Cannot access SoftHSM configuration file: ${TDX_SIGNED_HSM_PKCS11_SOFTHSM_CONF}"
        export SOFTHSM2_CONF="${TDX_SIGNED_HSM_PKCS11_SOFTHSM_CONF}"
    fi
}

tdx_signed_hsm_vars_export() {
    # common configuration
    tdx_signed_hsm_vars_export_common

    # OpenSSL engines directory
    for d in \
        "${STAGING_LIBDIR_NATIVE}/engines-3" \
        "${STAGING_LIBDIR_NATIVE}/engines-1.1"
    do
        if [ -d "$d" ]; then
            export OPENSSL_ENGINES="$d"
            break
        fi
    done
    [ -d "${OPENSSL_ENGINES}" ] || bbfatal "OpenSSL engines directory not found in native sysroot."

    # HSM token PIN
    export MKIMAGE_SIGN_PIN="${TDX_SIGNED_HSM_TOKEN_PIN}"

    # PKCS#11 module
    export PKCS11_MODULE_PATH="${RECIPE_SYSROOT_NATIVE}/${TDX_SIGNED_HSM_PKCS11_MODULE_PATH}"
    [ -r "${PKCS11_MODULE_PATH}" ] || bbfatal "PKCS#11 module missing or not readable: ${PKCS11_MODULE_PATH}"
}

# Python counterpart of tdx_signed_hsm_vars_export, for Python tasks
def tdx_signed_hsm_vars_export_py(d):
    import os

    pin = d.getVar('TDX_SIGNED_HSM_TOKEN_PIN')
    if not pin:
        bb.fatal("Missing PIN to access the HSM. Please configure it via TDX_SIGNED_HSM_TOKEN_PIN variable.")

    module = d.getVar('TDX_SIGNED_HSM_PKCS11_MODULE_PATH') or ''
    if 'libsofthsm2.so' in module:
        softhsm_conf = d.getVar('TDX_SIGNED_HSM_PKCS11_SOFTHSM_CONF')
        if not os.access(softhsm_conf, os.R_OK):
            bb.fatal("Cannot access SoftHSM configuration file: %s" % softhsm_conf)
        os.environ['SOFTHSM2_CONF'] = softhsm_conf

    for engines in ('engines-3', 'engines-1.1'):
        engines_dir = os.path.join(d.getVar('STAGING_LIBDIR_NATIVE'), engines)
        if os.path.isdir(engines_dir):
            os.environ['OPENSSL_ENGINES'] = engines_dir
            break
    else:
        bb.fatal("OpenSSL engines directory not found in native sysroot.")

    os.environ['MKIMAGE_SIGN_PIN'] = pin

    module_path = d.getVar('RECIPE_SYSROOT_NATIVE') + '/' + module
    if not os.access(module_path, os.R_OK):
        bb.fatal("PKCS#11 module missing or not readable: %s" % module_path)
    os.environ['PKCS11_MODULE_PATH'] = module_path

# Set up the environment so binman can sign TI K3 boot containers via a key stored in an HSM
tdx_signed_hsm_vars_export_k3_binman() {
    # common configuration
    tdx_signed_hsm_vars_export_common

    # binman x509_cert key URI
    if [ -z "${TDX_SIGNED_HSM_K3_BINMAN_KEY_URL}" ]; then
        bbfatal "Missing PKCS#11 URI for K3 binman signing. Please configure it via TDX_SIGNED_HSM_K3_BINMAN_KEY_URL variable."
    fi

    # OpenSSL configuration: use user-supplied file if available, otherwise generate one
    if [ -n "${TDX_SIGNED_HSM_K3_OPENSSL_CONF}" ]; then
        [ -r "${TDX_SIGNED_HSM_K3_OPENSSL_CONF}" ] || bbfatal "Cannot read OpenSSL configuration file: ${TDX_SIGNED_HSM_K3_OPENSSL_CONF}"
        export OPENSSL_CONF="${TDX_SIGNED_HSM_K3_OPENSSL_CONF}"
    else
        pkcs11_module="${RECIPE_SYSROOT_NATIVE}/${TDX_SIGNED_HSM_PKCS11_MODULE_PATH}"
        [ -r "${pkcs11_module}" ] || bbfatal "PKCS#11 module missing or not readable: ${pkcs11_module}"
        cat > "${WORKDIR}/openssl-pkcs11.cnf" <<EOF
openssl_conf = openssl_init

[openssl_init]
providers = provider_sect

[provider_sect]
default = default_sect
pkcs11 = pkcs11_sect

[default_sect]
activate = 1

[pkcs11_sect]
pkcs11-module-path = ${pkcs11_module}
pkcs11-module-quirks = no-deinit
activate = 1
EOF
        export OPENSSL_CONF="${WORKDIR}/openssl-pkcs11.cnf"
    fi

    # binman x509_cert key URI and PIN
    export BINMAN_X509_KEY_URI="${TDX_SIGNED_HSM_K3_BINMAN_KEY_URL}"
    export PKCS11_PIN="${TDX_SIGNED_HSM_TOKEN_PIN}"
}
