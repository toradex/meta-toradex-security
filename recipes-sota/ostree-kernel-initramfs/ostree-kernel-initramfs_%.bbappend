do_install:tdx-signed:intel-corei7-64() {
    kernelver="$(cat ${DEPLOY_DIR_IMAGE}/kernel-abiversion)"
    kerneldir=${D}${KERNEL_BUILD_ROOT}$kernelver
    install -d $kerneldir

    cp ${DEPLOY_DIR_IMAGE}/${OSTREE_KERNEL} $kerneldir/vmlinuz

    if "${@bb.utils.contains('KERNEL_CLASSES', 'kernel-fit-extra-artifacts', 'true', 'false', d)}" ||
        "${@bb.utils.contains('IMAGE_CLASSES', 'uki', 'true', 'false', d)}"; then
        if [ -n "${INITRAMFS_IMAGE}" ]; then
            # this is a hack for ostree not to override init= in kernel cmdline -
            # make it think that the initramfs is present (while it is in FIT image)
            touch $kerneldir/initramfs.img
        fi
    else
        if [ -n "${INITRAMFS_IMAGE}" ]; then
            cp ${DEPLOY_DIR_IMAGE}/${INITRAMFS_IMAGE}-${MACHINE}.${INITRAMFS_FSTYPES} $kerneldir/initramfs.img
        fi

        if [ ${@ oe.types.boolean('${OSTREE_DEPLOY_DEVICETREE}')} = True ] && [ -n "${OSTREE_DEVICETREE}" ]; then
            mkdir -p $kerneldir/dtb
            for dts_file in ${OSTREE_DEVICETREE}; do
                dts_file_basename=$(basename $dts_file)
                cp ${DEPLOY_DIR_IMAGE}/$dts_file_basename $kerneldir/dtb/$dts_file_basename
            done

            if [ ${@ oe.types.boolean('${OSTREE_MULTI_DEVICETREE_SUPPORT}')} = False ]; then
                cp $kerneldir/dtb/$(basename $(echo ${OSTREE_DEVICETREE} | awk '{print $1}')) $kerneldir/devicetree
            fi
        fi
    fi
}
