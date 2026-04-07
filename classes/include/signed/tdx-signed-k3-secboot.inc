# Enable Secure Boot on the TI K3 architecture:
TDX_K3_SECBOOT_ENABLE ?= "1"

# Define if the build is targeting a HS-FS (0) or HS-SE (1) device:
TDX_K3_SECBOOT_TARGET_HSSE_DEVICE ?= "0"

# Signing keys location.
TDX_K3_SECBOOT_KEY_DIR ?= "${TOPDIR}/keys/ti"

# Signing key name
# It can be a path to a PEM file or a PKCS#11 URI if signing via an HSM is enabled
TDX_K3_SECBOOT_KEY_NAME ?= "custMpk.pem"

# Whether to show warnings about the generated image with regards to
# variable TDX_K3_SECBOOT_TARGET_HSSE_DEVICE:
TDX_K3_SUPPRESS_HSSE_WARNINGS ?= "0"

# Choose between HSSE/HSFS variants of tiboot3.bin:
def target_hsse(d):
    return bb.utils.to_boolean(d.getVar('TDX_K3_SECBOOT_ENABLE')) and \
           bb.utils.to_boolean(d.getVar('TDX_K3_SECBOOT_TARGET_HSSE_DEVICE'))
SYSFW_SUFFIX := "${@'hs' if target_hsse(d) else 'hs-fs'}"

# Sanity check:
addhandler validate_k3_secboot_variables
validate_k3_secboot_variables[eventmask] = "bb.event.SanityCheck"
python validate_k3_secboot_variables() {
    # Check use of old variables:
    if e.data.getVar('TDX_K3_HSSE_ENABLE') or e.data.getVar('TDX_K3_HSSE_KEY_DIR'):
        bb.fatal('Variables TDX_K3_HSSE_ENABLE and TDX_K3_HSSE_KEY_DIR no longer exist; '
                 'please review your configuration. Check the documentation for the new '
                 'variables TDX_K3_SECBOOT_ENABLE, TDX_K3_SECBOOT_KEY_DIR and '
                 'TDX_K3_SECBOOT_TARGET_HSSE_DEVICE.')
}

# Informational messages:
addhandler show_tdx_signed_warnings
show_tdx_signed_warnings[eventmask] = "bb.event.BuildCompleted"
python show_tdx_signed_warnings() {
    if not bb.utils.to_boolean(e.data.getVar('TDX_K3_SUPPRESS_HSSE_WARNINGS')):
        if bb.utils.to_boolean(e.data.getVar('TDX_K3_SECBOOT_TARGET_HSSE_DEVICE')):
            bb.warnonce(
                'With the current value of TDX_K3_SECBOOT_TARGET_HSSE_DEVICE, '
                'the generated image is expected to run on a HS-SE device; '
                'therefore, make sure the device is properly fused, otherwise '
                'the image will hang at boot.')
        else:
            bb.warnonce(
                'With the current value of TDX_K3_SECBOOT_TARGET_HSSE_DEVICE, '
                'the generated image is expected to run on a HS-FS device, i.e. '
                'the device is supposed to be unfused; normally this setup is '
                'used only during development/evaluation.')
}
