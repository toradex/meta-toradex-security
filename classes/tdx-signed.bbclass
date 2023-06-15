# globally enable signed images
DISTROOVERRIDES:append = ":tdx-signed"

# FIT image configuration
require tdx-signed-fit-image.inc

# IXM HAB configuration
require tdx-signed-imx-hab.inc
