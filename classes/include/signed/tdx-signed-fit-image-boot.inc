# boot a FIT image instead of separate kernel, DTB and overlay files
TDX_KERNEL_FIT = "1"

# recipe that builds the FIT image
TDX_KERNEL_FIT_RECIPE ?= "linux-yocto-fitimage"

# deploy additional artifacts required for FIT image creation
KERNEL_CLASSES:append = "${@' kernel-fit-extra-artifacts' if bb.utils.to_boolean(d.getVar('TDX_KERNEL_FIT')) else ''}"

# make sure the FIT image is deployed before generating images
TDX_KERNEL_FIT_DEPLOY = "${@'${TDX_KERNEL_FIT_RECIPE}:do_deploy' if bb.utils.to_boolean(d.getVar('TDX_KERNEL_FIT')) else ''}"
KERNEL_DEPLOY_DEPEND:append = " ${TDX_KERNEL_FIT_DEPLOY}"
do_image_wic[depends] += "${TDX_KERNEL_FIT_DEPLOY}"
do_image_bootfs[depends] += "${TDX_KERNEL_FIT_DEPLOY}"

# make sure image recipes see KERNEL_IMAGETYPE='fitImage'
IMAGE_CLASSES += "${@'tdx-kernel-fit-boot' if bb.utils.to_boolean(d.getVar('TDX_KERNEL_FIT')) else ''}"

# The FIT image recipe must include linux-fitimage-sec-features.inc
python __anonymous() {
    if d.getVar('BPN') != d.getVar('TDX_KERNEL_FIT_RECIPE') or not bb.utils.to_boolean(d.getVar('TDX_KERNEL_FIT')):
        return

    if d.getVar('TDX_KERNEL_FIT_SEC_FEATURES') != '1':
        bb.fatal("%s is the FIT image recipe (TDX_KERNEL_FIT_RECIPE) but does not include "
                 "linux-fitimage-sec-features.inc. Add 'require recipes-kernel/linux/linux-fitimage-sec-features.inc' "
                 "to the recipe or to a bbappend of it." % d.getVar('BPN'))
}
