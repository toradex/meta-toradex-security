SUMMARY = "i.MX High Assurance Boot Reference Code Signing Tool"
DESCRIPTION = "NXP Code Signing Tool for the High Assurance Boot library. \
Provides software code signing support designed for use with i.MX processors \
that integrate the HAB library in the internal boot ROM"

LICENSE = "BSD-3-Clause"
LIC_FILES_CHKSUM = "file://COPYING;md5=dc56c17219895403ffc9aea66e228c8c"

DEPENDS = "openssl-native bison-native flex-native json-c-native"

SRC_URI = "\
    git://github.com/toradex/imx-code-signing-tool.git;protocol=https;branch=main \
"

SRCREV = "8931a2f536205a76c3f1778b02df857426291090"

S = "${WORKDIR}/git"
OECMAKE_SOURCEPATH = "${S}/src"

EXTRA_OECMAKE:append = "\
    -DCMAKE_C_FLAGS='${CFLAGS} -Wno-error=unused-result' \
"

inherit cmake native

require ${@oe.utils.conditional('TDX_IMX_HAB_CST_BUILD_WITH_PKCS11', '1', 'imx-code-signing-tool-native-pkcs11.inc', '', d)}
