# Add OP-TEE image node after the kernel image node
fitimage_emit_section_kernel:append() {
    cat << EOF >> $1
                optee-$2 {
                        description = "OP-TEE";
                        data = /incbin/("${DEPLOY_DIR_IMAGE}/tee.bin");
                        type = "${UBOOT_MKIMAGE_KERNEL_TYPE}";
                        arch = "${UBOOT_ARCH}";
                        os = "linux";
                        compression = "none";
                        load = <${TDX_OPTEE_ADDR_LOAD}>;
                        entry = <${TDX_OPTEE_ADDR_LOAD}>;
                        hash-1 {
                                algo = "${FIT_HASH_ALG}";
                        };
                };
EOF
}

# For each configuration node with a kernel, also add an OP-TEE correspondent node
# The configuration node will have 'optee_' appended to it. Example:
# conf-nxp_imx_imx6q-apalis-eval.dtb -> conf-optee_nxp_imx_imx6q-apalis-eval.dtb
fitimage_emit_section_config:append() {
    if [ -n "$kernel_id" ]; then
        cat << EOF >> $its_file
                ${FIT_CONF_PREFIX}optee_$dtb_image {
                        description = "$default_flag $conf_desc, OP-TEE";
                        $compatible_line
                        kernel = "optee-1";
                        loadables = "kernel-1";
                        $fdt_line
                        $ramdisk_line
                        $bootscr_line
                        $setup_line
                        hash-1 {
                                algo = "$conf_csum";
                        };
                        signature-1 {
                                algo = "$conf_csum,$conf_sign_algo";
                                key-name-hint = "$conf_sign_keyname";
                                padding = "$conf_padding_algo";
                                sign-images = "kernel", "fdt", "ramdisk", "loadables";
                        };
                };
EOF
    fi
}

# make sure OP-TEE image is deployed before generating the FIT image
do_assemble_fitimage[depends] += "optee-os:do_deploy"
