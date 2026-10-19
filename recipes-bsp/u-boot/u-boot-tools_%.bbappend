FILESEXTRAPATHS:prepend := "${THISDIR}/files:"

PKCS11_PATCH = "file://0001-lib-rsa-allow-matching-pkcs11-path-by-object-id.patch"

SRC_URI:append = "\
    ${@oe.utils.conditional('TDX_SIGNED_HSM', '1', '${PKCS11_PATCH}', '', d)} \
"
