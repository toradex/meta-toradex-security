require ${@ 'recipes-bsp/u-boot/u-boot-distro-boot-harden.inc' if 'tdx-signed' in d.getVar('OVERRIDES').split(':') else ''}

require ${@ 'recipes-bsp/u-boot/u-boot-distro-boot-optee.inc' if d.getVar('TDX_OPTEE_ENABLE') == '1' else ''}
