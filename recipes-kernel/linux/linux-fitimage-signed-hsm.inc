inherit tdx-signed-hsm-env

# Set up the FIT image signing for an HSM.
# mkimage reads the PIN, the PKCS#11 module and the OpenSSL engines from the
# environment, which is exported here, and oe.fitimage needs a workaround to
# accept a PKCS#11 URI as key directory (see below).
python tdx_signed_hsm_fit_setup() {
    import shlex
    import subprocess
    import oe.fitimage

    tdx_signed_hsm_vars_export_py(d)

    # oe.fitimage aborts when <keydir>/<keyname>.key and .crt do not exist,
    # which is always the case with a PKCS#11 URI. Skip that check as done by
    # oe-core commit b606b7153001 ("kernel-fit-image: Skip sign key check for
    # PKCS #11 URI"), not available in this release yet.
    root_class = oe.fitimage.ItsNodeRootKernel
    run_mkimage_sign = root_class.run_mkimage_sign

    def run_mkimage_sign_pkcs11(self, fitfile):
        if not self._sign_enable or not self._sign_keydir.startswith('pkcs11:'):
            return run_mkimage_sign(self, fitfile)

        cmd = [
            self._mkimage_sign,
            '-F',
            '-k', self._sign_keydir,
            '-r', fitfile
        ]
        if self._mkimage_dtcopts:
            cmd.extend(['-D', self._mkimage_dtcopts])
        if self._mkimage_sign_args:
            cmd.extend(shlex.split(self._mkimage_sign_args))
        try:
            subprocess.run(cmd, check=True, capture_output=True)
        except subprocess.CalledProcessError as e:
            bb.fatal(f"Command '{' '.join(cmd)}' failed with return code {e.returncode}\nstdout: {e.stdout.decode()}\nstderr: {e.stderr.decode()}")

    root_class.run_mkimage_sign = run_mkimage_sign_pkcs11
}

do_compile[prefuncs] += "tdx_signed_hsm_fit_setup"
