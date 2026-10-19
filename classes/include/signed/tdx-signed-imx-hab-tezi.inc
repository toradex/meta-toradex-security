TDX_IMX_HAB_GEN_UBOOT_FUSING_CMD ?= "0"

# add a function to U-Boot's environment to program the secure boot fuses
# this is mostly intended for development/tests, so it will only program
# the fuses but not close the device
add_fusing_cmd_to_uboot_env() {
    uboot_cmd="prog_secure_boot_fuses=echo Programming fuses...; setenv force_prog_ecc y"
    for line in $(grep "^H:F:" ${DEPLOY_DIR_IMAGE}/imx-config.fuse); do
        bank=$(echo "$line" | cut -d: -f3)
        word=$(echo "$line" | cut -d: -f4)
        value=$(echo "$line" | cut -d: -f5)
        uboot_cmd="${uboot_cmd}; fuse prog -y $bank $word $value"
    done

    uboot_env_file="${DEPLOY_DIR_IMAGE}/${UBOOT_ENV_TEZI}"
    if ! grep -q ^prog_secure_boot_fuses= ${uboot_env_file}; then
        echo $uboot_cmd >> ${uboot_env_file}
    else
        sed -i "s/^prog_secure_boot_fuses=.*/$uboot_cmd/" ${uboot_env_file}
    fi
}
TEZI_IMAGE_TEZIIMG_PREFUNCS:append = "${@ ' add_fusing_cmd_to_uboot_env' if d.getVar('TDX_IMX_HAB_GEN_UBOOT_FUSING_CMD') == '1' else ''}"
