# enable K3 HS-SE (High Security - Security Enforced)
TDX_K3_HSSE_ENABLE ?= "1"

# signing key location
TDX_K3_HSSE_KEY_DIR ?= "${TOPDIR}/keys/ti"

# build secure variant of tiboot3.bin
SYSFW_SUFFIX = "hs"

# use tiboot3.bin secure boot variant in Tezi
FIRMWARE_BINARY[0073] = "tiboot3-am62x-hs-verdin.bin"
FIRMWARE_BINARY[0074] = "tiboot3-am62x-hs-verdin.bin"
FIRMWARE_BINARY[0075] = "tiboot3-am62x-hs-verdin.bin"
FIRMWARE_BINARY[0076] = "tiboot3-am62x-hs-verdin.bin"
