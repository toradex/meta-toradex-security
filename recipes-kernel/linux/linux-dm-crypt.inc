SRC_URI:append = "\
    file://dm-crypt.cfg \
    file://trusted-keys.cfg \
    file://${TDX_ENC_KEY_BACKEND}.cfg \
"

# apply a patch on downstream based kernel to prevent an error
# when using the TPM as a source for the Trusted Keys subsystem
SRC_URI_TPM = "\
    file://0001-KEYS-trusted-revert-changes-introduced-by-NXP.patch \
"
SRC_URI:append:pn-linux-toradex = "${@ '${SRC_URI_TPM}' if d.getVar('TDX_ENC_KEY_BACKEND') == 'tpm' else ''}"
