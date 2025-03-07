SRC_URI:append = "\
    file://fit-signature.cfg \
"

require ${@ 'u-boot-fit-signature-nxp.inc' if 'imx-generic-bsp' in d.getVar('OVERRIDES').split(':') else ''}

require ${@ 'u-boot-fit-signature-ti.inc' if 'k3' in d.getVar('OVERRIDES').split(':') else ''}
