require ${@ 'u-boot-distro-boot-harden.inc' if 'tdx-signed' in d.getVar('OVERRIDES').split(':') else ''}
