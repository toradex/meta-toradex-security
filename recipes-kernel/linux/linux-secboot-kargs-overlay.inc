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

    # Generate overlay in the kernel tree (built from KERNEL_DEVICETREE):
    bbdebug 1 "Generating overlay file with secure boot bootargs..."
    local dtso="${S}/arch/${ARCH}/boot/dts/${KERNEL_DTB_PREFIX}${TDX_SECBOOT_KARGS_OVERLAY}.dtso"
    cat <<EOF >"${dtso}"
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
}

do_patch[postfuncs] += "gen_secboot_kargs"
