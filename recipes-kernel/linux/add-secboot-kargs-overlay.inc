gen_secboot_kargs() {
    if [ "${TDX_UBOOT_HARDENING_ENABLE}" != "1" ]; then
	bbdebug 1 "Overlay with bootargs will not be built because hardening is disabled."
	return 0
    fi

    if [ -z "${TDX_SECBOOT_KARGS_OVERLAY}" ]; then
	bbfatal "TDX_SECBOOT_KARGS_OVERLAY is not set."
    fi

    if [ -z "${TDX_SECBOOT_REQUIRED_BOOTARGS}" ]; then
	bbwarn "Required boot arguments are not set;" \
	       "variable TDX_SECBOOT_REQUIRED_BOOTARGS should be set to match the fixed part" \
	       "of the kernel command line as passed by the bootloader to the kernel."
    fi

    cd "${S}"

    # Generate overlay:
    bbdebug 1 "Generating overlay file with secure boot bootargs..."
    local dts="${TDX_SECBOOT_KARGS_OVERLAY}.dts"
    cat <<EOF >"${dts}"
/* Automatically generated file - do not edit. */

/dts-v1/;
/plugin/;

&{/} {
	chosen {
		toradex,secure-boot {
			required-bootargs = "${TDX_SECBOOT_REQUIRED_BOOTARGS}";
		};
	};
};
EOF

    # Add overlay to Makefile:
    local dtbo="${TDX_SECBOOT_KARGS_OVERLAY}.dtbo"
    local newrow="dtb-y += ${dtbo}"
    if ! grep -qF "dtb-y +=" Makefile; then
	bbfatal "Cannot patch device-tree-overlays Makefile!"
    fi
    if ! grep -qF "${newrow}"; then
	bbdebug 1 "Patching Makefile..."
        sed -i "$(grep -nF 'dtb-y +=' Makefile | tail -1 | cut -d':' -f1)a ${newrow}" Makefile
    else
        bbdebug 1 "Makefile already patched!"
    fi
}

do_patch[postfuncs] += "gen_secboot_kargs"
