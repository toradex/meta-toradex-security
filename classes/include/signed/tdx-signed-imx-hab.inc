# Enable iMX HAB/AHAB.
TDX_IMX_HAB_ENABLE ?= "1"

# additional configuration if tezi is enabled
require ${@ 'tdx-signed-imx-hab-tezi.inc' if 'teziimg' in d.getVar('IMAGE_FSTYPES') else ''}
