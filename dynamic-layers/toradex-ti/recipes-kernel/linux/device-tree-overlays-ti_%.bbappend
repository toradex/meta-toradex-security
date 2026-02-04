# device tree overlay for U-Boot hardening
require ${@ 'recipes-kernel/linux/add-secboot-kargs-overlay.inc' if 'tdx-signed' in d.getVar('OVERRIDES').split(':') else ''}

# device tree overlay for OP-TEE
require ${@ 'recipes-kernel/linux/add-optee-overlay.inc' if d.getVar('TDX_OPTEE_DT_OVERLAY') == '1' else ''}
