# globally enable signed images
DISTROOVERRIDES .= ":tdx-signed"

# FIT image configuration
require tdx-signed-fit-image.inc

# NXP i.MX HAB configuration
MACHINEOVERRIDES_EXTENDER ?= ""
require ${@ 'tdx-signed-imx-hab.inc' if 'imx-generic-bsp' in d.getVar('MACHINEOVERRIDES_EXTENDER').split(':') else ''}

# TI K3 secure boot configuration
require ${@ 'tdx-signed-k3-secboot.inc' if 'k3r5' in d.getVar('OVERRIDES').split(':') else ''}
require ${@ 'tdx-signed-k3-secboot.inc' if 'k3' in d.getVar('OVERRIDES').split(':') else ''}

# Hardening configuration
require tdx-signed-harden.inc
