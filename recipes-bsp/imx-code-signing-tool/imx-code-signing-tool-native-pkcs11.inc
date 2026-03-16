DEPENDS += "libp11-native patchelf-native"

EXTRA_OECMAKE:append = "\
    -DCST_WITH_PKCS11=ON \
    -Dlibp11_DIR=/usr/lib/engines-3 \
"

do_configure:prepend() {
    # CST's CMakeLists forces static OpenSSL (OPENSSL_USE_STATIC_LIBS=TRUE). This breaks PKCS#11/HSM use
    # because the PKCS#11 stack (engine_pkcs11/libp11 + vendor module like YKCS11/SoftHSM) is dynamically
    # linked to libcrypto.so, leading to two OpenSSL instances in the same process and CST crashes (SIGSEGV).
    # Force dynamic OpenSSL linking so CST and the PKCS#11 engine/module share the same libcrypto/libssl.
    sed -i 's/set(OPENSSL_USE_STATIC_LIBS TRUE)/set(OPENSSL_USE_STATIC_LIBS FALSE)/' \
        ${S}/src/CMakeLists.txt

    # CST hardcodes the PKCS#11 dependency to a static archive (libpkcs11.a). In OE, we may only have the
    # shared lib available; allow CMake to resolve either libpkcs11.so or libpkcs11.a by searching for "pkcs11"
    sed -i 's/find_cst_dependency_library("libpkcs11\.a"/find_cst_dependency_library("pkcs11"/' \
        ${S}/src/CMakeLists.txt
}

do_install:append() {
    # CST is linked against the OpenSSL pkcs11 engine (pkcs11.so), which lives in ${libdir}/engines-3.
    # Add that directory to RUNPATH so the dynamic loader can resolve pkcs11.so at startup.
    patchelf --set-rpath '$ORIGIN/../lib:$ORIGIN/../lib/engines-3:$ORIGIN/../../lib' ${D}${bindir}/cst
}
