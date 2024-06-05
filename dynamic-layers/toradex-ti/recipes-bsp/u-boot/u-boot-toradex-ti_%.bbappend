require ${@ 'recipes-bsp/u-boot/u-boot-secure-boot.inc' if 'tdx-signed' in d.getVar('OVERRIDES').split(':') else ''}
