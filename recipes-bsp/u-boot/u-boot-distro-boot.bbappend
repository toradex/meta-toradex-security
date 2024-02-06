require ${@ 'u-boot-distro-boot-harden.inc' if 'tdx-signed-bsp' in d.getVar('OVERRIDES').split(':') else ''}
