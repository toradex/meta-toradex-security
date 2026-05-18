# enable secure debug support
TDX_SECURE_DEBUG_ENABLE ?= "1"

# Secure-debug operating mode
#   authenticated - debug access requires authentication
#   disabled      - block security-sensitive debug access (not the same as
#                   full JTAG disable; some lower-risk features such as
#                   boundary scan may still remain available, depending on
#                   the SoC family)
TDX_SECURE_DEBUG_MODE ?= "authenticated"

# include backend configuration
MACHINEOVERRIDES_EXTENDER ?= ""
require ${@ 'include/secure-debug/tdx-secure-debug-sjc.inc' if 'mx8-generic-bsp' in d.getVar('MACHINEOVERRIDES_EXTENDER').split(':') else ''}

# Generic configuration validation
addhandler validate_secure_debug_generic
validate_secure_debug_generic[eventmask] = "bb.event.SanityCheck"
python validate_secure_debug_generic() {
    if e.data.getVar('TDX_SECURE_DEBUG_ENABLE') != '1':
        return

    mode = e.data.getVar('TDX_SECURE_DEBUG_MODE')
    if mode not in ('authenticated', 'disabled'):
        bb.fatal("TDX_SECURE_DEBUG_MODE must be one of: "
                 "authenticated, disabled (got '%s')." % mode)
}
