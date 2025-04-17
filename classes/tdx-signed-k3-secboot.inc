# Enable Secure Boot on the TI K3 architecture:
TDX_K3_SECBOOT_ENABLE ?= "1"

# Define if the build is targeting a HS-FS (0) or HS-SE (1) device:
TDX_K3_SECBOOT_TARGET_HSSE_DEVICE ?= "0"

# Signing keys location.
TDX_K3_SECBOOT_KEY_DIR ?= "${TOPDIR}/keys/ti"

# Use secure variant of tiboot3.bin:
require ${@'tdx-signed-k3-secboot-tiboot3.inc' if bb.utils.to_boolean(d.getVar("TDX_K3_SECBOOT_ENABLE")) and bb.utils.to_boolean(d.getVar("TDX_K3_SECBOOT_TARGET_HSSE_DEVICE")) else ''}
