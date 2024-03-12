# dm-verity support
require ${@ 'linux-dm-verity.inc' if 'tdx-signed-dmverity' in d.getVar('OVERRIDES').split(':') else ''}
