inherit tdx-signed-hsm-env

# Set up the FIT image signing for an HSM.
# mkimage reads the PIN, the PKCS#11 module and the OpenSSL engines from the
# environment, which is exported here.
python tdx_signed_hsm_fit_setup() {
    tdx_signed_hsm_vars_export_py(d)
}

do_compile[prefuncs] += "tdx_signed_hsm_fit_setup"
