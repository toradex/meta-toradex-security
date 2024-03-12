# enable K3 HS-SE (High Security - Security Enforced)
TDX_K3_HSSE_ENABLE ?= "1"

# signing key location
TDX_K3_HSSE_KEY_DIR ?= "${TOPDIR}/keys/ti"

# use secure variant of tiboot3.bin
require ${@oe.utils.conditional('TDX_K3_HSSE_ENABLE', '1', 'tdx-signed-k3-hs-se-tiboot3.inc', '', d)}
