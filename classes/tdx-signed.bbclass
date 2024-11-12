# globally enable signed images
DISTROOVERRIDES .= ":tdx-signed"

# FIT image configuration
require tdx-signed-fit-image.inc

# IXM HAB configuration
MACHINEOVERRIDES_EXTENDER ?= ""
require ${@ 'tdx-signed-imx-hab.inc' if 'imx-generic-bsp' in d.getVar('MACHINEOVERRIDES_EXTENDER').split(':') else ''}

# TI K3 HS-SE (High Security - Security Enforced) configuration
require ${@ 'tdx-signed-k3-hs-se.inc' if 'k3r5' in d.getVar('OVERRIDES').split(':') else ''}
require ${@ 'tdx-signed-k3-hs-se.inc' if 'k3' in d.getVar('OVERRIDES').split(':') else ''}

# Hardening configuration
require tdx-signed-harden.inc
