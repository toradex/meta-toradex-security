SRC_URI:append = "\
    file://optee.cfg \
    ${@oe.utils.conditional('TDX_OPTEE_FTPM', '1', 'file://optee-ftpm.cfg', '', d)} \
"

# add OP-TEE image and OP-TEE configuration nodes to FIT image
require ${@ 'linux-optee-fit.inc' if d.getVar('TDX_OPTEE_ADD_FIT_NODES') == '1' else ''}
