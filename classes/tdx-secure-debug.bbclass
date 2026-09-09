# globally enable secure debug support
DISTROOVERRIDES .= ":tdx-secure-debug"

# enable secure debug support
TDX_SECURE_DEBUG_ENABLE ?= "1"

# Secure-debug operating mode
#   authenticated - debug access requires authentication
#   no-debug      - block security-sensitive debug access (not the same as
#                   full JTAG disable; some lower-risk features such as
#                   boundary scan may still remain available, depending on
#                   the SoC family)
TDX_SECURE_DEBUG_MODE ?= "authenticated"

# Machines the selected backend has been validated on. Each backend appends
# the machines it supports; an empty list means no backend was selected for
# this target, which is also the case for SoC families that have no backend
# implemented yet.
TDX_SECURE_DEBUG_SUPPORTED_MACHINES ?= ""

# include backend configuration
MACHINEOVERRIDES_EXTENDER ?= ""
require ${@ 'include/secure-debug/tdx-secure-debug-sjc.inc' if 'mx6-generic-bsp' in d.getVar('MACHINEOVERRIDES_EXTENDER').split(':') else ''}
require ${@ 'include/secure-debug/tdx-secure-debug-sjc.inc' if 'mx7-generic-bsp' in d.getVar('MACHINEOVERRIDES_EXTENDER').split(':') else ''}
require ${@ 'include/secure-debug/tdx-secure-debug-sjc.inc' if 'mx8m-generic-bsp' in d.getVar('MACHINEOVERRIDES_EXTENDER').split(':') else ''}

# Generic configuration validation
addhandler validate_secure_debug_generic
validate_secure_debug_generic[eventmask] = "bb.event.SanityCheck"
python validate_secure_debug_generic() {
    if e.data.getVar('TDX_SECURE_DEBUG_ENABLE') != '1':
        return

    machine = e.data.getVar('MACHINE')
    supported = (e.data.getVar('TDX_SECURE_DEBUG_SUPPORTED_MACHINES') or '').split()
    if machine not in supported:
        bb.fatal("Secure Debug is currently not supported on '%s' machine!" % machine)

    mode = e.data.getVar('TDX_SECURE_DEBUG_MODE')
    if mode not in ('authenticated', 'no-debug'):
        bb.fatal("TDX_SECURE_DEBUG_MODE must be one of: "
                 "authenticated, no-debug (got '%s')." % mode)
}
