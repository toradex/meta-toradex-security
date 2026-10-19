SUMMARY = "YubiKey PIV tool, libraries (libykpiv) and PKCS#11 module (libykcs11)"
DESCRIPTION = "The Yubico PIV tool is used for interacting with the Personal Identity \
Verification (PIV) application on a YubiKey. With it you may generate keys on the device, \
importing keys and certificates, and create certificate requests, and other operations. \
A shared library and a command-line tool is included."
HOMEPAGE = "https://developers.yubico.com/yubico-piv-tool/"
SECTION = "security"

LICENSE = "BSD-2-Clause"
LIC_FILES_CHKSUM = "file://COPYING;md5=b83cd346ac9e78624518062f20fdeebe"

DEPENDS = "openssl pcsc-lite libcheck gengetopt-native"

SRC_URI = "git://github.com/Yubico/yubico-piv-tool;protocol=https;branch=master"
SRCREV = "ed1cd7862d39a92502c0476f53dfcf93f195007a"

S = "${WORKDIR}/git"

EXTRA_OECMAKE += " \
    -DGENERATE_MAN_PAGES=OFF \
    -DBUILD_TESTING=OFF \
"

inherit cmake pkgconfig

BBCLASSEXTEND = "native nativesdk"
