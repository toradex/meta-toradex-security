MODSIGN_DEV_KEYDIR = "${META_TORADEX_SECURITY_LAYERDIR}/keys/modsign-dev"
MODSIGN_KEYDIR ?= "${MODSIGN_DEV_KEYDIR}"
MODSIGN_KEY_NAME ?= "key-module"
MODSIGN_KEY_FILES = "${MODSIGN_KEY_NAME}.key ${MODSIGN_KEY_NAME}.crt"
MODSIGN_KEY_FILE_CHECKSUMS = "${@' '.join('%s/%s:True' % (d.getVar('MODSIGN_KEYDIR'), key) for key in d.getVar('MODSIGN_KEY_FILES').split())}"

modsign_prepare_key() {
    if [ -z "${MODSIGN_KEYDIR}" ]; then
        bbfatal "Kernel module signing key directory is not configured."
    fi

    for key in ${MODSIGN_KEY_FILES}; do
        [ -f "${MODSIGN_KEYDIR}/${key}" ] || \
            bbfatal "Kernel module signing key is missing: ${MODSIGN_KEYDIR}/${key}"
    done

    if [ "${MODSIGN_KEYDIR}" = "${MODSIGN_DEV_KEYDIR}" ] && \
       [ -z "${modsign_dev_key_warning_emitted}" ]; then
        bbwarn "Kernel module development key from ${MODSIGN_DEV_KEYDIR} is being used. This public test credential must not sign production images. Set MODSIGN_KEYDIR to a production key directory."
        modsign_dev_key_warning_emitted=1
    fi

    cat "${MODSIGN_KEYDIR}/${MODSIGN_KEY_NAME}.key" \
        "${MODSIGN_KEYDIR}/${MODSIGN_KEY_NAME}.crt" > "${B}/modsign_key.pem"
}
