# Add the OP-TEE image node, and for each configuration node with a kernel, an
# OP-TEE configuration node that boots OP-TEE and loads the kernel. The name of
# the new node has 'optee_' prepended to the DTB name. Example:
# conf-nxp_imx_imx6q-apalis-eval.dtb -> conf-optee_nxp_imx_imx6q-apalis-eval.dtb
#
# The FIT recipe builds the ITS through oe.fitimage, which has no hook for
# extra nodes, so wrap its configuration section emitter for this task.
python tdx_optee_fit_add_nodes() {
    import oe.fitimage

    root_class = oe.fitimage.ItsNodeRootKernel
    emit_section_config = root_class.fitimage_emit_section_config

    tee_path = os.path.join(d.getVar('DEPLOY_DIR_IMAGE'), 'tee.bin')
    tee_load = d.getVar('TDX_OPTEE_ADDR_LOAD')
    image_type = d.getVar('UBOOT_MKIMAGE_KERNEL_TYPE')

    def emit_section_config_optee(self, *args, **kwargs):
        optee = self.its_add_node_image('optee-1', 'OP-TEE', image_type, 'none', {
            'data': '/incbin/("%s")' % tee_path,
            'arch': self._arch,
            'os': 'linux',
            'load': '<%s>' % tee_load,
            'entry': '<%s>' % tee_load,
        })

        emit_section_config(self, *args, **kwargs)

        for conf in list(self.configurations.sub_nodes):
            if 'kernel' not in conf.properties:
                continue

            props = dict(conf.properties)
            description = props.pop('description') + ', OP-TEE'
            props['kernel'] = optee.name
            props['loadables'] = [self._kernel.name] + props.get('loadables', [])

            name = self._conf_prefix + 'optee_' + conf.name[len(self._conf_prefix):]
            optee_conf = oe.fitimage.ItsNodeConfiguration(name, self.configurations, description, opt_props=props)

            # copy the hash and signature nodes, also signing the loadables
            for sub_node in conf.sub_nodes:
                sub_props = dict(sub_node.properties)
                if 'sign-images' in sub_props and 'loadables' not in sub_props['sign-images']:
                    sub_props['sign-images'] = sub_props['sign-images'] + ['loadables']
                oe.fitimage.ItsNode(sub_node.name, optee_conf, None, sub_props)

    root_class.fitimage_emit_section_config = emit_section_config_optee
}

do_compile[prefuncs] += "tdx_optee_fit_add_nodes"

# make sure OP-TEE image is deployed before generating the FIT image
do_compile[depends] += "optee-os:do_deploy"
