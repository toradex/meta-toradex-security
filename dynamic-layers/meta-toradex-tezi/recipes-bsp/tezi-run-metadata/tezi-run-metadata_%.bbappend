# secure boot fixups to the Tezi recovery metadata
require ${@ 'tezi-run-metadata-secboot.inc' if 'tdx-signed' in d.getVar('OVERRIDES').split(':') else ''}
