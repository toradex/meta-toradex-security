SRC_URI:append = "\
    file://fit-signature.cfg \
"

SRC_URI:append:imx95-19x19-verdin = "\
    file://fit-signature-imx95-19x19-verdin.cfg \
"

require ${@ 'u-boot-fit-signature-nxp.inc' if 'imx-generic-bsp' in d.getVar('OVERRIDES').split(':') else ''}

require ${@ 'u-boot-fit-signature-ti.inc' if 'k3' in d.getVar('OVERRIDES').split(':') else ''}

require ${@oe.utils.conditional('TDX_SIGNED_HSM', '1', 'u-boot-fit-signature-hsm.inc', '', d)}
