# globally enable signed images
DISTROOVERRIDES .= ":tdx-signed"

# FIT image configuration
require include/signed/tdx-signed-fit-image.inc

# NXP i.MX HAB configuration
MACHINEOVERRIDES_EXTENDER ?= ""
require ${@ 'include/signed/tdx-signed-imx-hab.inc' if 'imx-generic-bsp' in d.getVar('MACHINEOVERRIDES_EXTENDER').split(':') else ''}

# TI K3 secure boot configuration
require ${@ 'include/signed/tdx-signed-k3-secboot.inc' if 'k3r5' in d.getVar('OVERRIDES').split(':') else ''}
require ${@ 'include/signed/tdx-signed-k3-secboot.inc' if 'k3' in d.getVar('OVERRIDES').split(':') else ''}

# Hardening configuration
require include/signed/tdx-signed-harden.inc

# machine-specific fixups for the signed image
include include/signed/machine/${MACHINE}.inc
