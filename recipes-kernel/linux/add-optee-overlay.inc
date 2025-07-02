FILESEXTRAPATHS:prepend := "${THISDIR}/files:"

SRC_URI:append = "\
    file://optee_overlay.dts \
"

gen_optee_dt_overlay() {
    if [ "${TDX_OPTEE_ENABLE}" != "1" ]; then
        bbfatal "Will not generate DT overlay since OP-TEE is not enabled!"
    fi

    if [ -z "${TDX_OPTEE_ADDR_LOAD}" ]; then
        bbfatal "Missing TDX_OPTEE_ADDR_LOAD to setup DT overlay!"
    fi

    if [ -z "${TDX_OPTEE_ADDR_SIZE}" ]; then
        bbfatal "Missing TDX_OPTEE_ADDR_SIZE to setup DT overlay!"
    fi

    sed -i 's|@@TDX_OPTEE_ADDR_LOAD@@|${TDX_OPTEE_ADDR_LOAD}|g' ${WORKDIR}/optee_overlay.dts
    sed -i 's|@@TDX_OPTEE_ADDR_SIZE@@|${TDX_OPTEE_ADDR_SIZE}|g' ${WORKDIR}/optee_overlay.dts

    cp ${WORKDIR}/optee_overlay.dts ${S}/${MACHINE_PREFIX}_optee_overlay.dts
}

do_patch[postfuncs] += "gen_optee_dt_overlay"
