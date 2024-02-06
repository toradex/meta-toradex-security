# globally enable signed images
DISTROOVERRIDES:append = ":tdx-signed-bsp"

# FIT image configuration
require tdx-signed-fit-image.inc

# IXM HAB configuration
require ${@ 'tdx-signed-imx-hab.inc' if 'imx-generic-bsp' in d.getVar('OVERRIDES').split(':') else ''}

# Hardening configuration
require tdx-signed-harden.inc
