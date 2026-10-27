# marks that the FIT image recipe includes this file (checked at parse time)
TDX_KERNEL_FIT_SEC_FEATURES = "1"

# op-tee support
require ${@ 'linux-fitimage-optee.inc' if d.getVar('TDX_OPTEE_ADD_FIT_NODES') == '1' else ''}
