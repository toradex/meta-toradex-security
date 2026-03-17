TDX_UBOOT_HARDENING_PATCHES_COMMON = "\
    file://0001-toradex-common-add-shared-hardening-modules.patch \
    file://0002-toradex-dts-add-fragment-file-to-configure-secure-bo.patch \
"

# TODO: Add "cp" protection to NXP downstream U-Boot.
# TODO: Add "fastboot" protection to NXP downstream U-Boot.
TDX_UBOOT_HARDENING_PATCHES_DOWNSTREAM = "\
    ${TDX_UBOOT_HARDENING_PATCHES_COMMON} \
    file://0001-toradex-integrate-command-whitelisting-downstream.patch \
    file://0002-toradex-integrate-bootm-protection-downstream.patch \
    file://0003-toradex-integrate-CLI-access-protection-downstream.patch \
    file://0004-toradex-integrate-bootargs-protection-downstream.patch \
    file://0005-toradex-integrate-load-protection-downstream.patch \
    file://0006-toradex-cmd-fuse-add-switch-for-quiet-operation-down.patch \
    file://0007-toradex-cmd-fuse-add-readv-command-downstream.patch \
    file://0008-toradex-cmd-setexpr-integrate-protections-downstream.patch \
"

# TODO: Add "cp" protection to TI downstream U-Boot.
# TODO: Add "fastboot" protection to TI downstream U-Boot.
TDX_UBOOT_HARDENING_PATCHES_DOWNSTREAM_K3 = "\
    ${TDX_UBOOT_HARDENING_PATCHES_COMMON} \
    file://0001-toradex-integrate-command-whitelisting-k3.patch \
    file://0002-toradex-integrate-bootm-protection-downstream.patch \
    file://0003-toradex-integrate-CLI-access-protection-downstream.patch \
    file://0004-toradex-integrate-bootargs-protection-k3.patch \
    file://0005-toradex-integrate-load-protection-k3.patch \
    file://0006-toradex-cmd-fuse-add-switch-for-quiet-operation-down.patch \
    file://0007-toradex-cmd-fuse-add-readv-command-downstream.patch \
    file://0008-toradex-cmd-setexpr-integrate-protections-downstream.patch \
"

TDX_UBOOT_HARDENING_PATCHES_UPSTREAM = "\
    ${TDX_UBOOT_HARDENING_PATCHES_COMMON} \
    file://0001-toradex-integrate-command-whitelisting-upstream.patch \
    file://0002-toradex-integrate-bootm-protection-upstream.patch \
    file://0003-toradex-integrate-CLI-access-protection-upstream.patch \
    file://0004-toradex-integrate-bootargs-protection-upstream.patch \
    file://0005-toradex-integrate-load-protection-upstream.patch \
    file://0006-toradex-integrate-cp-protection-upstream.patch \
    file://0007-toradex-integrate-fastboot-protection-upstream.patch \
    file://0008-toradex-cmd-fuse-add-switch-for-quiet-operation-upst.patch \
    file://0009-toradex-cmd-fuse-add-readv-command-upstream.patch \
    file://0010-toradex-cmd-setexpr-integrate-protections-upstream.patch \
"

TDX_UBOOT_HARDENING_PATCHES = "${TDX_UBOOT_HARDENING_PATCHES_UPSTREAM}"
TDX_UBOOT_HARDENING_PATCHES:apalis-imx8 = "${TDX_UBOOT_HARDENING_PATCHES_DOWNSTREAM}"
TDX_UBOOT_HARDENING_PATCHES:colibri-imx8x = "${TDX_UBOOT_HARDENING_PATCHES_DOWNSTREAM}"
TDX_UBOOT_HARDENING_PATCHES:toradex-smarc-imx95 = "${TDX_UBOOT_HARDENING_PATCHES_DOWNSTREAM}"
TDX_UBOOT_HARDENING_PATCHES:verdin-imx95 = "${TDX_UBOOT_HARDENING_PATCHES_DOWNSTREAM}"
TDX_UBOOT_HARDENING_PATCHES:k3 = "${TDX_UBOOT_HARDENING_PATCHES_DOWNSTREAM_K3}"
TDX_UBOOT_HARDENING_PATCHES:k3r5 = ""

SRC_URI:append = "\
    file://u-boot-harden.cfg \
    ${@oe.utils.conditional('TDX_UBOOT_HARDENING_ENABLE_DBG', '1', 'file://u-boot-harden-dbg.cfg', '', d)} \
    ${@oe.utils.conditional('TDX_UBOOT_HARDENING_ENABLE_CP_PROT', '1', 'file://u-boot-harden-cp-prot.cfg', '', d)} \
    ${@oe.utils.conditional('TDX_UBOOT_HARDENING_ENABLE_FB_PROT', '1', 'file://u-boot-harden-fb-prot.cfg', '', d)} \
    ${TDX_UBOOT_HARDENING_PATCHES} \
"

TDX_SECBOOT_WL_ALLOW_OPEN_CATEG ?= ""
TDX_SECBOOT_WL_ALLOW_CLOSED_CATEG ?= ""
TDX_SECBOOT_WL_DENY_OPEN_CATEG ?= ""
TDX_SECBOOT_WL_DENY_CLOSED_CATEG ?= ""
TDX_SECBOOT_WL_NEEDED_CATEG ?= ""
# TODO: Consider new variable to configure 'enable-cli-when-closed' property.

inherit cml1

TDX_SECBOOT_DEVICE_TREE_INCLUDES = "tdx-secboot.dtsi"

do_patch[postfuncs] += "amend_secboot_dtsi"
amend_secboot_dtsi_cell_prop() {
    prop="$1"
    value="$2"
    dtsi="${S}/arch/arm/dts/${TDX_SECBOOT_DEVICE_TREE_INCLUDES}"

    if ! grep -q "${prop} = <.*>" "${dtsi}"; then
        bbwarn "Property '${prop}' will not be honored because it is not present in '${dtsi}'"
	return 1
    fi

    bbnote "Amending '${prop}' property in '${dtsi}'"
    sed -i -Ee "s#^([[:space:]]*)(// )?(${prop} = )<.*>#\1\3<${value}>#" "${dtsi}"
}

amend_secboot_dtsi() {
    if [ -n "${TDX_SECBOOT_WL_ALLOW_OPEN_CATEG}" ]; then
        amend_secboot_dtsi_cell_prop \
	    "allow-open" "${TDX_SECBOOT_WL_ALLOW_OPEN_CATEG}"
    fi

    if [ -n "${TDX_SECBOOT_WL_ALLOW_CLOSED_CATEG}" ]; then
        amend_secboot_dtsi_cell_prop \
	    "allow-closed" "${TDX_SECBOOT_WL_ALLOW_CLOSED_CATEG}"
    fi

    if [ -n "${TDX_SECBOOT_WL_DENY_OPEN_CATEG}" ]; then
        amend_secboot_dtsi_cell_prop \
	    "deny-open" "${TDX_SECBOOT_WL_DENY_OPEN_CATEG}"
    fi

    if [ -n "${TDX_SECBOOT_WL_DENY_CLOSED_CATEG}" ]; then
        amend_secboot_dtsi_cell_prop \
	    "deny-closed" "${TDX_SECBOOT_WL_DENY_CLOSED_CATEG}"
    fi

    if [ -n "${TDX_SECBOOT_WL_NEEDED_CATEG}" ]; then
        amend_secboot_dtsi_cell_prop \
	    "needed" "${TDX_SECBOOT_WL_NEEDED_CATEG}"
    fi
}

do_configure:append() {
    if [ -n "${UBOOT_CONFIG}" ]; then
        unset i j
        for config in ${UBOOT_MACHINE}; do
            i=$(expr $i + 1);
            for type in ${UBOOT_CONFIG}; do
                j=$(expr $j + 1);
                if [ $j -eq $i ]; then
                    extend_device_tree_includes_config $config $type
                fi
            done
            unset j
        done
        unset i
    else
        extend_device_tree_includes
    fi
}

_make_dt_includes_fragment() {
    config_file=$1
    frag_file=$2

    # Get original includes (if any):
    incs=""
    if grep -q '^CONFIG_DEVICE_TREE_INCLUDES=' "${config_file}"; then
        incs=$(sed -n -e 's/^CONFIG_DEVICE_TREE_INCLUDES="\(.*\)"/\1/p' "${config_file}")
    fi
    bbdebug 1 "Original value of CONFIG_DEVICE_TREE_INCLUDES='${incs}' in '${config_file}'"

    # Append the desired includes:
    [ -n "${incs}" ] && incs="${incs} "
    incs="${incs}${TDX_SECBOOT_DEVICE_TREE_INCLUDES}"

    bbdebug 1 "Updated value of CONFIG_DEVICE_TREE_INCLUDES='${incs}'"
    bbdebug 1 "Writing fragment file into '${frag_file}'"

    echo "CONFIG_DEVICE_TREE_INCLUDES=\"${incs}\"" > "${frag_file}"
}

extend_device_tree_includes_config() {
    config=$1
    type=$2

    frag="${WORKDIR}/.add-dt-includes-${type}.cfg"
    _make_dt_includes_fragment "${B}/${config}/.config" "${frag}"

    merge_config.sh -m -O "${B}/${config}" "${B}/${config}/.config" "${frag}"
    oe_runmake -C "${S}" O="${B}/${config}" oldconfig
}

extend_device_tree_includes() {
    frag="${WORKDIR}/.add-dt-includes.cfg"
    _make_dt_includes_fragment ".config" "${frag}"

    merge_config.sh -m ".config" "${frag}"
    cml1_do_configure
}

TDX_CHECK_UNSUPP_BOOTMETH ?= "1"
do_configure[postfuncs] += "\
    ${@oe.utils.conditional('TDX_CHECK_UNSUPP_BOOTMETH', '1', 'check_dotconfig_compat_all', '', d)} \
"
check_dotconfig_compat_all() {
    if [ -n "${UBOOT_CONFIG}" ]; then
        unset i j
        for config in ${UBOOT_MACHINE}; do
            i=$(expr $i + 1);
            for type in ${UBOOT_CONFIG}; do
                j=$(expr $j + 1);
                if [ $j -eq $i ]; then
                    check_dotconfig_compat $config $type
                fi
            done
            unset j
        done
        unset i
    else
        check_dotconfig_compat
    fi
}

check_dotconfig_compat() {
    config=$1
    dotconfig="${B}/${config}/.config"

    # TODO: Consider performing the checks only if CONFIG_BOOTSTD=y (Standard Boot enabled).
    # NOTE: Currently only the "Script" bootmeth is supported.
    bootmeth_allow='^\(CONFIG_BOOTMETH_SCRIPT\)$'
    bootmeth_inuse=$(cat "${dotconfig}" | \
                     sed -ne 's#^\(CONFIG_BOOTMETH_[A-Z_0-9]\+\)=y#\1#p')
    bootmeth_forbidden=$(echo "${bootmeth_inuse}" | sed "/${bootmeth_allow}/d")
    # Drop spaces:
    bootmeth_forbidden=$(echo ${bootmeth_forbidden})

    if [ -n "${bootmeth_forbidden}" ]; then
        bbfatal "The config option(s) [${bootmeth_forbidden}] are enabled in" \
                "U-Boot but are not supported by the hardening."
    fi
}

TDX_DISABLE_UNSUPP_BOOTMETH ?= "1"
SRC_URI:append = "\
    ${@oe.utils.conditional('TDX_DISABLE_UNSUPP_BOOTMETH', '1', 'file://disable-bootmeths.cfg', '', d)} \
"

CP_PROT_SUPPORTED ?= "0"
CP_PROT_SUPPORTED:verdin-imx8mm ?= "1"
CP_PROT_SUPPORTED:verdin-imx8mp ?= "1"

FB_PROT_SUPPORTED ?= "0"
FB_PROT_SUPPORTED:verdin-imx8mm ?= "1"
FB_PROT_SUPPORTED:verdin-imx8mp ?= "1"

do_compile:prepend() {
    if [ "${TDX_UBOOT_HARDENING_ENABLE_DBG}" != "0" ]; then
        bbwarn 'Debugging of the hardening feature is enabled; be sure to disable it for production.'
    fi
    if [ "${TDX_UBOOT_HARDENING_ENABLE_CP_PROT}" != "0" ]; then
        if [ "${CP_PROT_SUPPORTED}" != "1" ]; then
            bbfatal '"cp" command protection is not supported/tested on the current machine' \
                    '(set CP_PROT_SUPPORTED="1" to bypass this error check at your own risk).'
        fi
        bbwarn 'Hardening of the "cp" command is enabled; do this only when strictly required.'
    fi
    if [ "${TDX_UBOOT_HARDENING_ENABLE_FB_PROT}" != "0" ]; then
        if [ "${FB_PROT_SUPPORTED}" != "1" ]; then
            bbfatal 'Fastboot protection is not supported/tested on the current machine' \
                    '(set FB_PROT_SUPPORTED="1" to bypass this error check at your own risk).'
        fi
        bbwarn 'Hardening of Fastboot is enabled; do this only when strictly required.'
    fi
}

do_compile:prepend:imx-generic-bsp() {
    if [ "${TDX_IMX_HAB_ENABLE}" = "0" ] && [ "${TDX_UBOOT_HARDENING_ENABLE}" = "1" ]; then
        bbfatal 'The combination TDX_IMX_HAB_ENABLE="0" and TDX_UBOOT_HARDENING_ENABLE="1" is not allowed:' \
                'the whitelisting feature (part of the hardening) currently relies on HAB/AHAB.'
    fi
}

do_compile:prepend:k3() {
    if [ "${TDX_K3_SECBOOT_ENABLE}" = "0" ] && [ "${TDX_UBOOT_HARDENING_ENABLE}" = "1" ]; then
        bbfatal 'The combination TDX_K3_SECBOOT_ENABLE="0" and TDX_UBOOT_HARDENING_ENABLE="1" is not allowed:' \
                'the whitelisting feature (part of the hardening) currently relies on K3.'
    fi
}

do_compile:prepend:k3r5() {
    # TODO: Consider implementing some kind of hardening for the R5 processor.
    if [ "${TDX_UBOOT_HARDENING_ENABLE}" = "1" ]; then
        bbfatal 'The hardening cannot be enabled for the R5 processor at the moment;' \
	        'please review your setting of TDX_UBOOT_HARDENING_ENABLE.'
    fi
}
