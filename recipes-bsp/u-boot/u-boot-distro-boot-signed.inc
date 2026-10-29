require ${@ 'recipes-bsp/u-boot/u-boot-distro-boot-signed-fit.inc' if bb.utils.to_boolean(d.getVar('TDX_KERNEL_FIT') or '0') else ''}

require recipes-bsp/u-boot/u-boot-distro-boot-signed-harden.inc
