# pkcs11-provider-native is used by recipes in this layer to sign artifacts
# via PKCS#11 (e.g. binman signing the TI K3 boot container with an HSM-backed
# key). The upstream recipe does not declare a native variant, so enable it here.
BBCLASSEXTEND += "native"
