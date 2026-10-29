SRC_URI:append = "\
    file://optee.cfg \
    ${@oe.utils.conditional('TDX_OPTEE_FTPM', '1', 'file://optee-ftpm.cfg', '', d)} \
"
