require ${@ 'add-secboot-kargs-overlay.inc' if 'tdx-signed' in d.getVar('OVERRIDES').split(':') else ''}
