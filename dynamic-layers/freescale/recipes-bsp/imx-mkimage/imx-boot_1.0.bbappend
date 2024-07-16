require ${@oe.utils.conditional('TDX_IMX_HAB_ENABLE', '1', 'imx-boot-hab.inc', '', d)}
