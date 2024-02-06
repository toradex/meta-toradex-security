require ${@ 'add-secboot-kargs-overlay.inc' if 'tdx-signed-bsp' in d.getVar('OVERRIDES').split(':') else ''}
