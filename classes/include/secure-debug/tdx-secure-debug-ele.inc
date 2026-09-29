# Secure Debug - ELE (EdgeLock Secure Enclave) backend
# Implements secure debug for NXP iMX9x based SoCs

# Machines this backend has been validated on
TDX_SECURE_DEBUG_SUPPORTED_MACHINES:append = "\
    aquila-imx95 \
    toradex-smarc-imx95 \
    verdin-imx95 \
"

# Fuse map template describing the debug fuses of this SoC
TDX_SECURE_DEBUG_ELE_FUSE_TEMPLATE ?= ""
TDX_SECURE_DEBUG_ELE_FUSE_TEMPLATE:mx95-generic-bsp = "imx95-ele-template.fuse"
TDX_SECURE_DEBUG_FUSE_TEMPLATE = "${TDX_SECURE_DEBUG_ELE_FUSE_TEMPLATE}"

# SoC family name used by the NXP SPSDK tools
TDX_SECURE_DEBUG_ELE_SPSDK_FAMILY ?= ""
TDX_SECURE_DEBUG_ELE_SPSDK_FAMILY:mx95-generic-bsp = "mimx9596"

# Bits of the debug credential cc_socu field that select OEM debug domains
# (one nibble per domain) on this SoC. The other bits select NXP domains,
# which cannot be opened with OEM keys
TDX_SECURE_DEBUG_ELE_OEM_DOMAINS ?= ""
TDX_SECURE_DEBUG_ELE_OEM_DOMAINS:mx95-generic-bsp = "0x00FFFFF0"

# Debug domains opened by the generated debug credential (cc_socu field),
# used when TDX_SECURE_DEBUG_MODE = "authenticated". Defaults to all OEM
# debug domains of the SoC with all debug permissions. The encoding
# of the field is described in docs/README-secure-debug-ele.md
TDX_SECURE_DEBUG_ELE_DC_CC_SOCU ?= "${TDX_SECURE_DEBUG_ELE_OEM_DOMAINS}"

# Fully disable JTAG by programming the SoC's JTAG disable fuse (named
# SJC_DISABLE by NXP), in addition to the debug-disable fuses of all debug
# domains. When set to "1", this overrides TDX_SECURE_DEBUG_MODE and even
# boundary scan and the authenticated debug flow become unavailable.
#   0 - do nothing
#   1 - request full JTAG disable
TDX_SECURE_DEBUG_ELE_JTAG_DISABLE ?= "0"

# UUID of the device the generated debug credential is bound to, as 32 hex
# characters. When empty, the credential template carries a placeholder to be
# replaced with the UUID of each device. An all-zero UUID makes the credential
# valid for every device fused with the same secure boot keys.
TDX_SECURE_DEBUG_ELE_DC_UUID ?= ""

# Private key of the debug credential key (DCK) pair. The public part is
# embedded in the debug credential, and the private key signs the response
# to the challenge of each debug session.
TDX_SECURE_DEBUG_ELE_DCK_KEY ?= "${TOPDIR}/keys/secure-debug/dck.pem"

# ELE configuration validation
addhandler validate_secure_debug_ele
validate_secure_debug_ele[eventmask] = "bb.event.SanityCheck"
python validate_secure_debug_ele() {
    if e.data.getVar('TDX_SECURE_DEBUG_ENABLE') != '1':
        return

    machine = e.data.getVar('MACHINE')
    supported = (e.data.getVar('TDX_SECURE_DEBUG_SUPPORTED_MACHINES') or '').split()
    if machine not in supported:
        return

    if not e.data.getVar('TDX_SECURE_DEBUG_ELE_FUSE_TEMPLATE'):
        bb.fatal("TDX_SECURE_DEBUG_ELE_FUSE_TEMPLATE is not set for '%s'; "
                 "a Secure Debug fuse map is required for this SoC." % machine)

    if e.data.getVar('TDX_IMX_HAB_ENABLE') != '1':
        bb.fatal("TDX_SECURE_DEBUG_ENABLE=1 requires secure boot to be enabled")

    if e.data.getVar('TDX_SECURE_DEBUG_MODE') != 'authenticated' or \
            e.data.getVar('TDX_SECURE_DEBUG_ELE_JTAG_DISABLE') == '1':
        return

    crypto = e.data.getVar('TDX_IMX_HAB_CST_CRYPTO')
    srk_ca = e.data.getVar('TDX_IMX_HAB_CST_SRK_CA')
    if crypto != 'ecdsa' or srk_ca != '0':
        bb.warn("Authenticated debug may not work with the configured secure boot keys "
                "(TDX_IMX_HAB_CST_CRYPTO = \"%s\", TDX_IMX_HAB_CST_SRK_CA = \"%s\"). "
                "See the Limitations section of docs/README-secure-debug-ele.md."
                % (crypto, srk_ca))

    oem_domains = int(e.data.getVar('TDX_SECURE_DEBUG_ELE_OEM_DOMAINS'), 16)
    cc_socu = e.data.getVar('TDX_SECURE_DEBUG_ELE_DC_CC_SOCU')
    try:
        cc_socu_val = int(cc_socu, 16)
    except ValueError:
        bb.fatal("TDX_SECURE_DEBUG_ELE_DC_CC_SOCU must be a hexadecimal value "
                 "(got '%s')." % cc_socu)
    if cc_socu_val == 0 or cc_socu_val & ~oem_domains:
        bb.fatal("TDX_SECURE_DEBUG_ELE_DC_CC_SOCU must select at least one OEM "
                 "debug domain and no other bits (valid bits on '%s': 0x%08X; "
                 "got '%s')." % (machine, oem_domains, cc_socu))

    uuid = e.data.getVar('TDX_SECURE_DEBUG_ELE_DC_UUID')
    if uuid:
        digits = uuid[2:] if uuid.lower().startswith('0x') else uuid
        if len(digits) != 32 or not all(c in '0123456789abcdefABCDEF' for c in digits):
            bb.fatal("TDX_SECURE_DEBUG_ELE_DC_UUID must contain exactly 32 hex "
                     "characters (got '%s')." % uuid)
}
