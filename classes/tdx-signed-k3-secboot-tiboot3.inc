# build secure variant of tiboot3.bin
SYSFW_SUFFIX = "hs"

# include machine-specific tiboot3 configuration
include tdx-signed-k3-secboot-tiboot3-${MACHINE}.inc
