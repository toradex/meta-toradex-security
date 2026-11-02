FILESEXTRAPATHS:prepend := "${THISDIR}/files:"

# dm-verity support
require ${@ 'linux-dm-verity.inc' if 'tdx-signed-dmverity' in d.getVar('OVERRIDES').split(':') else ''}

# dm-crypt support
require ${@ 'linux-dm-crypt.inc' if 'tdx-encrypted' in d.getVar('OVERRIDES').split(':') else ''}

# op-tee support
require ${@ 'linux-optee.inc' if d.getVar('TDX_OPTEE_ENABLE') == '1' else ''}

# device tree overlay for U-Boot hardening
require ${@ 'linux-secboot-kargs-overlay.inc' if 'tdx-signed' in d.getVar('OVERRIDES').split(':') else ''}

# device tree overlay for OP-TEE
require ${@ 'linux-optee-overlay.inc' if d.getVar('TDX_OPTEE_DT_OVERLAY') == '1' else ''}
