FILESEXTRAPATHS:prepend := "${THISDIR}/files:"

# dm-verity support
require ${@ 'linux-dm-verity.inc' if 'tdx-signed-dmverity' in d.getVar('OVERRIDES').split(':') else ''}

# dm-crypt support
require ${@ 'linux-dm-crypt.inc' if d.getVar('TDX_ENC_ENABLE') == '1' else ''}
