require ${@ 'recipes-bsp/imx-system-manager/imx-sm-toradex-secure-boot.inc' if 'tdx-signed' in d.getVar('OVERRIDES').split(':') else ''}
