# Secure Debug - SJC (System JTAG Controller) backend
# Implements secure debug for NXP iMX6/iMX7/iMX8M based SoCs.

# Machines this backend has been validated on.
TDX_SECURE_DEBUG_SUPPORTED_MACHINES:append = "\
    apalis-imx6 \
    colibri-imx6 \
    colibri-imx6ull-emmc \
    colibri-imx7-emmc \
    toradex-smarc-imx8mp \
    verdin-imx8mm \
    verdin-imx8mp \
"

# Fuse map template describing where the SJC fields live on this SoC.
TDX_SECURE_DEBUG_SJC_FUSE_TEMPLATE ?= ""
TDX_SECURE_DEBUG_SJC_FUSE_TEMPLATE:mx6dl-generic-bsp = "imx6-sjc-template.fuse"
TDX_SECURE_DEBUG_SJC_FUSE_TEMPLATE:mx6q-generic-bsp = "imx6-sjc-template.fuse"
TDX_SECURE_DEBUG_SJC_FUSE_TEMPLATE:mx6ull-generic-bsp = "imx6-sjc-template.fuse"
TDX_SECURE_DEBUG_SJC_FUSE_TEMPLATE:mx7d-generic-bsp = "imx7-sjc-template.fuse"
TDX_SECURE_DEBUG_SJC_FUSE_TEMPLATE:mx8mm-generic-bsp = "imx8mm-sjc-template.fuse"
TDX_SECURE_DEBUG_SJC_FUSE_TEMPLATE:mx8mp-generic-bsp = "imx8mp-sjc-template.fuse"

# Block the HAB software path (HAB_JDE) that can re-enable JTAG from
# trusted boot software, bypassing the Secure JTAG challenge/response.
# Relevant only when TDX_SECURE_DEBUG_MODE = "authenticated".
#   0 - allow the HAB software path that may re-enable JTAG
#   1 - block the HAB software path that may re-enable JTAG
TDX_SECURE_DEBUG_SJC_HEO ?= "1"

# Fully disable JTAG by programming the SoC's SJC disable control.
# When set to "1", this overrides TDX_SECURE_DEBUG_MODE and selects the
# strongest available full-disable setting for the SoC: even boundary
# scan and other lower-risk JTAG paths are cut, in contrast to
# TDX_SECURE_DEBUG_MODE = "no-debug" which leaves those paths available.
#   0 - do nothing
#   1 - request full JTAG disable
TDX_SECURE_DEBUG_SJC_DISABLE ?= "0"

# Key file used to answer the secure-debug challenge/response mechanism,
# used when TDX_SECURE_DEBUG_MODE = "authenticated".
#
# Key length by SoC family:
#   - i.MX6 / i.MX7 / i.MX8MM : 56 bits => 14 hex characters
#
# File format:
#   - hexadecimal text
#   - no "0x" prefix
#   - no spaces or separators
#   - trailing newline allowed
TDX_SECURE_DEBUG_KEY_FILE ?= "${TOPDIR}/keys/secure-debug/key.txt"

# SJC configuration validation
addhandler validate_secure_debug_sjc
validate_secure_debug_sjc[eventmask] = "bb.event.SanityCheck"
python validate_secure_debug_sjc() {
    if e.data.getVar('TDX_SECURE_DEBUG_ENABLE') != '1':
        return

    machine = e.data.getVar('MACHINE')
    supported = (e.data.getVar('TDX_SECURE_DEBUG_SUPPORTED_MACHINES') or '').split()
    if machine not in supported:
        return

    if not e.data.getVar('TDX_SECURE_DEBUG_SJC_FUSE_TEMPLATE'):
        bb.fatal("TDX_SECURE_DEBUG_SJC_FUSE_TEMPLATE is not set for '%s'; "
                 "a Secure Debug fuse map is required for this SoC."
                 % e.data.getVar('MACHINE'))

    if e.data.getVar('TDX_IMX_HAB_ENABLE') != '1':
        bb.fatal("TDX_SECURE_DEBUG_ENABLE=1 requires secure boot to be enabled")

    # check key file used to answer the secure debug challenge/response mechanism
    if e.data.getVar('TDX_SECURE_DEBUG_MODE') == 'authenticated' and \
            e.data.getVar('TDX_SECURE_DEBUG_SJC_DISABLE') != '1':
        overrides = (e.data.getVar('MACHINEOVERRIDES') or '').split(':')
        if 'mx8mp-generic-bsp' in overrides:
            bb.fatal("Secure debug in authenticated mode is not available on "
                     "the iMX8MP. NXP defeatured the Secure JTAG mode on this "
                     "SoC (erratum ERR052318). Set TDX_SECURE_DEBUG_MODE="
                     "\"no-debug\" or TDX_SECURE_DEBUG_SJC_DISABLE=\"1\" to fully "
                     "disable JTAG.")

        key_file = e.data.getVar('TDX_SECURE_DEBUG_KEY_FILE')
        if not key_file:
            bb.fatal("TDX_SECURE_DEBUG_KEY_FILE must be set when "
                     "TDX_SECURE_DEBUG_MODE='authenticated'.")
        if not os.path.exists(key_file):
            bb.fatal("Secure debug key file not found: '%s'" % key_file)
        with open(key_file, 'r') as f:
            key = f.read().strip()
        if len(key) != 14:
            bb.fatal("Key file '%s' must contain exactly 14 hex characters "
                     "(56 bits); got %d." % (key_file, len(key)))
        if not all(c in '0123456789abcdefABCDEF' for c in key):
            bb.fatal("Key file '%s' must contain only hex characters." % key_file)
}
