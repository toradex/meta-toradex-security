SRC_URI:append = "\
    file://ele-dc.yaml.in \
    file://ele-dat.yaml.in \
"

# Secure debug is enabled only if both tdx-secure-debug class is inherited and TDX_SECURE_DEBUG_ENABLE="1"
TDX_SECURE_DEBUG_ENABLE_INTERNAL = "0"
TDX_SECURE_DEBUG_ENABLE_INTERNAL:tdx-secure-debug = "${TDX_SECURE_DEBUG_ENABLE}"

# Make sure fuse-cmds.txt is regenerated if a secure debug authentication key changes
do_compile[file-checksums] += "${@ d.getVar('TDX_SECURE_DEBUG_KEY_FILE') + ':True' if d.getVar('TDX_SECURE_DEBUG_KEY_FILE') and d.getVar('TDX_SECURE_DEBUG_ENABLE_INTERNAL') == '1' and d.getVar('TDX_SECURE_DEBUG_MODE') == 'authenticated' and d.getVar('TDX_SECURE_DEBUG_SJC_DISABLE') != '1' else ''}"
do_compile[file-checksums] += "${@ d.getVar('TDX_SECURE_DEBUG_TZ_KEY_FILE') + ':True' if d.getVar('TDX_SECURE_DEBUG_TZ_KEY_FILE') and d.getVar('TDX_SECURE_DEBUG_ENABLE_INTERNAL') == '1' and d.getVar('TDX_SECURE_DEBUG_MODE') == 'authenticated' and d.getVar('TDX_SECURE_DEBUG_SJC_DISABLE') != '1' else ''}"

# Enabled when authenticated debug is used on ELE-based devices
TDX_SECURE_DEBUG_ELE_AUTH_INTERNAL = "${@ '1' if d.getVar('TDX_SECURE_DEBUG_ENABLE_INTERNAL') == '1' and d.getVar('TDX_SECURE_DEBUG_ELE_SPSDK_FAMILY') and d.getVar('TDX_SECURE_DEBUG_MODE') == 'authenticated' and d.getVar('TDX_SECURE_DEBUG_ELE_JTAG_DISABLE') != '1' else '0'}"

# return the SRK private key path or the SPSDK PKCS#11 signer (if HSM is enabled)
def tdx_secure_debug_srk_signer(d):
    import os
    import urllib.parse
    if d.getVar('TDX_SIGNED_HSM') != '1':
        name = os.path.basename(make_srk_cert_name(d, '')).replace('_crt.pem', '_key.pem')
        return os.path.join(d.getVar('TDX_IMX_HAB_CST_DIR'), 'keys', name)
    uri = d.getVar('TDX_IMX_HAB_CST_SRK_CERT') or ''
    attrs = dict(a.split('=', 1) for a in uri.split(':', 1)[-1].split(';') if '=' in a)
    if not uri.startswith('pkcs11:') or 'token' not in attrs or \
            ('object' not in attrs and 'id' not in attrs):
        bb.fatal("TDX_IMX_HAB_CST_SRK_CERT must be a PKCS#11 URI with a 'token' attribute "
                 "and an 'object' or 'id' attribute when TDX_SIGNED_HSM=1 (got '%s')." % uri)
    # the SPSDK PKCS#11 signer selects the key by label or by hexadecimal ID
    if 'object' in attrs:
        key = 'key_label=%s' % urllib.parse.unquote(attrs['object'])
    else:
        key = 'key_id=%s' % urllib.parse.unquote_to_bytes(attrs['id']).hex()
    return 'type=pkcs11;so_path=%s;user_pin=$SPSDK_HSM_PIN;token_label=%s;%s' % (
        d.getVar('TDX_SIGNED_HSM_PKCS11_MODULE_PATH'),
        urllib.parse.unquote(attrs['token']), key)

# generate the SPSDK configuration files (dc.yaml and dat.yaml) for authenticated debug
python secure_debug_ele_templates() {
    import os
    import re
    import shutil

    workdir = d.getVar('WORKDIR')
    outdir = os.path.join(workdir, 'secure-debug')
    shutil.rmtree(outdir, ignore_errors=True)
    if d.getVar('TDX_SECURE_DEBUG_ELE_AUTH_INTERNAL') != '1':
        return

    # paths, PKCS#11 labels and other free-form values are written as
    # single-quoted YAML strings, so characters such as '#' are kept
    def quote(value):
        return "'%s'" % value.replace("'", "''")

    uuid = (d.getVar('TDX_SECURE_DEBUG_ELE_DC_UUID') or '').lower()
    uuid = uuid[2:] if uuid.startswith('0x') else uuid
    srk_cert = make_srk_cert_name(d, d.getVar('TDX_IMX_HAB_CST_CERTS_DIR'))
    srk_cert_suffix = re.sub(r'^SRK[1-4]_', '', os.path.basename(srk_cert))

    subst = {
        'FAMILY': quote(d.getVar('TDX_SECURE_DEBUG_ELE_SPSDK_FAMILY')),
        'CC_SOCU': quote('0x%08X' % int(d.getVar('TDX_SECURE_DEBUG_ELE_DC_CC_SOCU'), 16)),
        'UUID': quote(uuid or 'REPLACE_WITH_DEVICE_UUID'),
        'DCK_KEY': quote(d.getVar('TDX_SECURE_DEBUG_ELE_DCK_KEY')),
        'SRK_SIGNER': quote(tdx_secure_debug_srk_signer(d)),
        'SRK_ID': str(int(d.getVar('TDX_IMX_HAB_CST_SRK_INDEX')) - 1),
        'SRK_CA': 'true' if d.getVar('TDX_IMX_HAB_CST_SRK_CA') == '1' else 'false',
    }
    for i in range(1, 5):
        subst['SRK_CERT_%d' % i] = quote(os.path.join(os.path.dirname(srk_cert),
                                                      'SRK%d_%s' % (i, srk_cert_suffix)))

    # generate the configuration files from the templates
    os.makedirs(outdir)
    for tmpl in ('dc', 'dat'):
        with open(os.path.join(d.getVar('UNPACKDIR'), 'ele-%s.yaml.in' % tmpl)) as f:
            text = f.read()
        for name, value in subst.items():
            text = text.replace('@%s@' % name, value)
        with open(os.path.join(outdir, '%s.yaml' % tmpl), 'w') as f:
            f.write(text)
}
do_compile[postfuncs] += "secure_debug_ele_templates"

do_deploy:append() {
    if [ -d ${WORKDIR}/secure-debug ]; then
        install -d ${DEPLOYDIR}/secure-debug
        install -m 0644 ${WORKDIR}/secure-debug/*.yaml ${DEPLOYDIR}/secure-debug
    fi
}
