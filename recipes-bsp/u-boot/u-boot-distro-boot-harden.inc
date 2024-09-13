# When hardening is enabled, ask for the use of the overlay containing the
# "secure" bootargs. Here we assume an underlying layer will provide this
# capabitlity of applying predefined overlays inside the FIT image based on
# variable FITCONF_FDT_OVERLAYS; such capability would normally rely on the
# boot script.
#
# For Torizon OS, the handling of predefined overlays is done by the
# u-boot-distro-boot recipe provided by layer meta-toradex-torizon, whereas
# for Toradex BSP Reference images, this is done by the u-boot-distro-boot
# recipe provided by layer meta-toradex-bsp-common.
#
FITCONF_FDT_OVERLAYS .= "${@oe.utils.conditional('TDX_UBOOT_HARDENING_ENABLE', '1', '#conf-${TDX_SECBOOT_KARGS_OVERLAY}.dtbo', '', d)}"

# Whether or not to amend the boot script (this only works with the Toradex-
# provided boot script for BSP images); users utilizing a custom boot script
# would likely set this variable to "0".
TDX_AMEND_BOOT_SCRIPT ?= "${TDX_UBOOT_HARDENING_ENABLE}"

# Whether the baudrate is missing from the U-Boot console environment variable.
# When it is missing we append the baudrate variable into the console one when
# amending the boot script.
CONSOLE_BAUDRATE_MISSING ?= "1"

amend_console_argument() {
    # Look for a block like this:.
    # > # Set console baudrate only when required
    # > setexpr _res sub "," "_" ${console}
    # > if test ! $? -eq 0; then
    # >    env set console ...
    # > fi
    # And get rid of it. This is done to avoid the use of the "setexpr" command
    # which we prefer to keep outside the hardening command whitelist.
    match="$(sed -n -e '/^# Set console baudrate only when required/,+5p' boot.cmd | \
    	         sed -e '1d' \
                     -e '2{s/^setexpr.*/OK/}' \
	             -e '3{s/^if test.*$/OK/}' \
	             -e '4{s/^\s*env set console.*$/OK/}' \
	             -e '5{s/^fi\s*$/OK/}' \
	             -e '6{s/^\s*$/OK/}')"
    if [ "$(echo $match)" != "OK OK OK OK OK" ]; then
        bbfatal "Amendments to bootscript must be reviewed: block setting the 'console' variable has changed."
    fi

    sed -e '/^# Set console baudrate only when required/,+5d' \
        -i boot.cmd

    # Look for a line like this:
    # > env set setupargs ...console=${console}
    # That we may modify depending on CONSOLE_BAUDRATE_MISSING.
    if ! grep -q '^ *env set setupargs.*\bconsole=' boot.cmd; then
        bbfatal "Amendments to bootscript must be reviewed: line setting the 'setupargs' variable has not been found."
    fi

    if [ "${CONSOLE_BAUDRATE_MISSING}" = "1" ]; then
        sed -e '/env set setupargs .*\bconsole=\$''{console}\($''\|\s\)/{s#\bconsole\b=\$''{console}#console=\$''{console},$''{baudrate}#}' \
            -i boot.cmd
    fi
}

amend_root_argument() {
    # The goal of the following script is to turn a line like this:
    #
    # env set rootfsargs_set 'env set rootfsargs "  sarg1=sarg1 darg1=$darg1   darg2=$darg2    sarg2=sarg2"'
    #
    # into this:
    #
    # env set rootfsargs_set 'env set rootfsargs1 "sarg1=sarg1 sarg2=sarg2" && env set rootfsargs2 "darg1=$darg1 darg2=$darg2"'
    #
    # i.e. it breaks down rootfsargs into static (rootfsargs1) and dynamic
    # arguments (rootfsargs2).
    sed -e "
        /env set rootfsargs_set/ {
            s/^\(.*\)env set rootfsargs \"\(.*\)\"\('.*\)$/\2 \nS:\nD:\n\1\n\3/
            :l1
            s/^ \+//; t l1
            s/^\([^\n \$]\+\) \(.*\nS:[^\n]*\)\(.*\)$/\2 \1\3/; t l1
            s/^\([^\n \$]*\$[^\n ]\+\) \(.*\nD:[^\n]*\)\(.*\)$/\2 \1\3/; t l1
            s/.*\nS: *\([^\n]*\)\nD: *\([^\n]*\)\n\([^\n]*\)\n\([^\n]*\)/\3env set rootfsargs1 \"\1\" \&\& env set rootfsargs2 \"\2\"\4/
        }" \
    	-i boot.cmd

    # Use the splitted rootfsargs keeping the static part at the beginning and
    # the dynamic part at the end of the bootargs variable. For this, the following
    # script will turn:
    #
    # env set bootcmd_args 'run rootfsargs_set && env set bootargs ${rootfsargs} ${setupargs} ${appendargs} ${tdxargs}'
    #                                                              ^^^^^^^^^^^^^
    # into:
    #
    # env set bootcmd_args 'run rootfsargs_set && env set bootargs ${rootfsargs1} ${setupargs} ${appendargs} ${tdxargs} ${rootfsargs2}'
    #                                                              ^^^^^^^^^^^^^^                                       ^^^^^^^^^^^^^^
    sed -e '
        /^env set bootcmd_args/ {
            s/env set bootargs \$''{rootfsargs} \$''{setupargs} \$''{appendargs} \$''{tdxargs}/env set bootargs \$''{rootfsargs1} \$''{setupargs} \$''{appendargs} \$''{tdxargs} \$''{rootfsargs2}/
        }' \
    	-i boot.cmd

    if [ "$(grep 'rootfsargs[^12_]' boot.cmd | wc -l)" -ne 0 ]; then
        bbfatal "Amendments to bootscript must be reviewed: there are unhandled occurrences of variable 'rootfsargs'."
    fi
}

do_compile:append() {
    if [ "${TDX_AMEND_BOOT_SCRIPT}" = "1" ]; then
        amend_console_argument
        amend_root_argument
    fi
}
