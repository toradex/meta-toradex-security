require ${@ 'u-boot-secure-boot.inc' if 'tdx-signed' in d.getVar('OVERRIDES').split(':') else ''}

require ${@oe.utils.conditional('TDX_OPTEE_ENABLE', '1', 'u-boot-optee.inc', '', d)}
