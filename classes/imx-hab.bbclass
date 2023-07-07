# NXP CST Tool
# Cannot be automatically downloaded because it requires a registration
# Download from https://www.nxp.com/webapp/sps/download/license.jsp?colCode=IMX_CST_TOOL_NEW
TDX_IMX_HAB_CST_DIR ?= "${TOPDIR}/keys/cst"
TDX_IMX_HAB_CST_BIN ?= "${TDX_IMX_HAB_CST_DIR}/linux64/bin/cst"

# certificates and keys
TDX_IMX_HAB_CST_KEY_SIZE  ?= "2048"
TDX_IMX_HAB_CST_DIG_ALGO  ?= "sha256"
TDX_IMX_HAB_CST_CERTS_DIR ?= "${TDX_IMX_HAB_CST_DIR}/crts"
TDX_IMX_HAB_CST_SRK       ?= "${TDX_IMX_HAB_CST_CERTS_DIR}/SRK_1_2_3_4_table.bin"
TDX_IMX_HAB_CST_SRK_FUSE  ?= "${TDX_IMX_HAB_CST_CERTS_DIR}/SRK_1_2_3_4_fuse.bin"
TDX_IMX_HAB_CST_CSF_CERT  ?= "${TDX_IMX_HAB_CST_CERTS_DIR}/CSF1_1_${TDX_IMX_HAB_CST_DIG_ALGO}_${TDX_IMX_HAB_CST_KEY_SIZE}_65537_v3_usr_crt.pem"
TDX_IMX_HAB_CST_IMG_CERT  ?= "${TDX_IMX_HAB_CST_CERTS_DIR}/IMG1_1_${TDX_IMX_HAB_CST_DIG_ALGO}_${TDX_IMX_HAB_CST_KEY_SIZE}_65537_v3_usr_crt.pem"
TDX_IMX_HAB_CST_SRK_CERT  ?= "${TDX_IMX_HAB_CST_CERTS_DIR}/SRK1_${TDX_IMX_HAB_CST_DIG_ALGO}_${TDX_IMX_HAB_CST_KEY_SIZE}_65537_v3_usr_crt.pem"
