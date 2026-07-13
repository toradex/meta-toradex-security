require ${@ 'classes/include/signed/tdx-signed-uefi-secboot.inc' if 'tdx-signed' in d.getVar('OVERRIDES').split(':') else ''}
