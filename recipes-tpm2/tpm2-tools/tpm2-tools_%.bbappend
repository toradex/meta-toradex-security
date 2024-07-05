# we talk directly to the TPM device and don't use the daemon
DEPENDS:remove = "tpm2-abrmd"
RDEPENDS:${PN} = "libtss2 libtss2-tcti-device"
