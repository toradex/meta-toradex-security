require ${@oe.utils.conditional('TDX_OPTEE_ENABLE', '1', 'optee-client-tdx.inc', '', d)}
