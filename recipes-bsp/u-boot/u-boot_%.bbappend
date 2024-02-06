require ${@ 'u-boot-secure-boot.inc' if 'tdx-signed-bsp' in d.getVar('OVERRIDES').split(':') else ''}
