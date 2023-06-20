# NXP CST Tool
# Cannot be automatically downloaded because it requires a registration
# Download from https://www.nxp.com/webapp/sps/download/license.jsp?colCode=IMX_CST_TOOL_NEW
TDX_IMX_HAB_CST_DIR ?= "${TOPDIR}/keys/cst"
TDX_IMX_HAB_CST_BIN ?= "${TDX_IMX_HAB_CST_DIR}/linux64/bin/cst"

# certificates and keys
TDX_IMX_HAB_CST_CERTS_DIR ?= "${TOPDIR}/keys/cst"
TDX_IMX_HAB_CST_SRK       ?= "${TDX_IMX_HAB_CST_CERTS_DIR}/crts/SRK_1_2_3_4_table.bin"
TDX_IMX_HAB_CST_SRK_FUSE  ?= "${TDX_IMX_HAB_CST_CERTS_DIR}/crts/SRK_1_2_3_4_fuse.bin"
TDX_IMX_HAB_CST_CSF_CERT  ?= "${TDX_IMX_HAB_CST_CERTS_DIR}/crts/CSF1_1_sha256_2048_65537_v3_usr_crt.pem"
TDX_IMX_HAB_CST_IMG_CERT  ?= "${TDX_IMX_HAB_CST_CERTS_DIR}/crts/IMG1_1_sha256_2048_65537_v3_usr_crt.pem"
