# NXP CST Tool
# Cannot be automatically downloaded because it requires a registration
# Download from https://www.nxp.com/webapp/sps/download/license.jsp?colCode=IMX_CST_TOOL_NEW
TDX_IMX_HAB_CST_DIR ?= "${TOPDIR}/keys/cst"
TDX_IMX_HAB_CST_BIN ?= "${TDX_IMX_HAB_CST_DIR}/linux64/bin/cst"

#
# Main variables that control the automatic certificate names generation; users
# will likely want to change these; they should match the answers/parameters
# given to the CST tool when generating the keys and certificates.
#
# Explanation:
#
# - TDX_IMX_HAB_CST_CRYPTO: Type of cryptographic keys in use; allowed values
#   are "rsa" or "ecdsa". One should set this to "ecdsa" only if the option to
#   use "Elliptic Curve Cryptography" was selected during key generation.
#
# - TDX_IMX_HAB_CST_KEY_SIZE: For RSA keys, this would be the key length (in
#   bits) as entered into the CST tool. For ECDSA, this would be a string
#   determined from the generated certificate file name; for example, for a file
#   named "SRK1_sha256_secp384r1_v3_ca_crt.pem" (found in the certificates
#   directory) the present variable would be set to "secp384r1".
#
# - TDX_IMX_HAB_CST_DIG_ALGO: Digest algorithm as entered into the CST tool.
#
# - TDX_IMX_HAB_CST_SRK_CA: Whether or not the SRK certificates have the CA flag
#   set as entered into the CST tool; allowed values are "0" or "1"
#
TDX_IMX_HAB_CST_CRYPTO    ?= "rsa"
TDX_IMX_HAB_CST_KEY_SIZE  ?= "2048"
TDX_IMX_HAB_CST_DIG_ALGO  ?= "sha256"
TDX_IMX_HAB_CST_SRK_CA    ?= "1"

#
# Secondary parameters for automaticaly generating certificate file names:
#
# Explanation:
#
# - TDX_IMX_HAB_CST_KEY_INDEX: Zero-based index of the SRK to be used within the
#   SRK table. NOTE: This is not fully handled yet so it should be set to 0.
# - TDX_IMX_HAB_CST_KEY_EXP: Key exponent for RSA keys (only).
#

# TODO: Handle the key index when generating the CSF files for HABv4 and AHAB.
TDX_IMX_HAB_CST_KEY_INDEX ?= "0"
TDX_IMX_HAB_CST_KEY_EXP   ?= "65537"

#
# Helper functions
#
def make_srk_cert_name(d, basedir):
    """Generate certificate name related to a Super Root Key"""
    res = ""
    crypto = d.getVar("TDX_IMX_HAB_CST_CRYPTO")
    kidx = int(d.getVar("TDX_IMX_HAB_CST_KEY_INDEX"))
    dalgo = d.getVar("TDX_IMX_HAB_CST_DIG_ALGO")
    ksize = d.getVar("TDX_IMX_HAB_CST_KEY_SIZE")
    caflg = int(d.getVar("TDX_IMX_HAB_CST_SRK_CA"))
    castr = "ca" if caflg != 0 else "usr"
    if kidx < 0 or kidx > 3:
        bb.fatal("TDX_IMX_HAB_CST_KEY_INDEX must be in the range [0,3]")
    if crypto == "rsa":
        kexp = d.getVar("TDX_IMX_HAB_CST_KEY_EXP")
        res = f"SRK{kidx+1}_{dalgo}_{ksize}_{kexp}_v3_{castr}_crt.pem"
    elif crypto == "ecdsa":
        if ksize.isdigit():
            bb.warn("TDX_IMX_HAB_CST_KEY_SIZE is likely not set correctly;"
                    "check the documentation to understand how to set this variable for ECDSA.")
        res = f"SRK{kidx+1}_${dalgo}_${ksize}_v3_{castr}_crt.pem"
    else:
        bb.fatal('TDX_IMX_HAB_CST_CRYPTO is not set correctly'
                 '(its value must be either "rsa" or "ecdsa")')
    if res:
        res = os.path.join(basedir, res)
    return res

def make_sub_cert_name(d, prefix, basedir):
    """Generate certificate name related to a subordinate key"""
    res = ""
    crypto = d.getVar("TDX_IMX_HAB_CST_CRYPTO")
    kidx = int(d.getVar("TDX_IMX_HAB_CST_KEY_INDEX"))
    dalgo = d.getVar("TDX_IMX_HAB_CST_DIG_ALGO")
    ksize = d.getVar("TDX_IMX_HAB_CST_KEY_SIZE")
    caflg = int(d.getVar("TDX_IMX_HAB_CST_SRK_CA"))
    if kidx < 0 or kidx > 3:
        bb.fatal("TDX_IMX_HAB_CST_KEY_INDEX must be in the range [0,3]")
    if caflg == 0:
        # Subordinate keys/certs only exist when the SRK cert has the CA flag set.
        pass
    elif crypto == "rsa":
        kexp = d.getVar("TDX_IMX_HAB_CST_KEY_EXP")
        res = f"{prefix}{kidx+1}_1_{dalgo}_{ksize}_{kexp}_v3_usr_crt.pem"
    elif crypto == "ecdsa":
        if ksize.isdigit():
            bb.warn("TDX_IMX_HAB_CST_KEY_SIZE is likely not set correctly;"
                    "check the documentation to understand how to set this variable for ECDSA.")
        res = f"{prefix}{kidx+1}_1_{dalgo}_{ksize}_v3_usr_crt.pem"
    else:
        bb.fatal('TDX_IMX_HAB_CST_CRYPTO is not set correctly'
                 '(its value must be either "rsa" or "ecdsa")')
    if res:
        res = os.path.join(basedir, res)
    return res

#
# Lower level variables (users don't usually need to modify these directly)
#
# Explanation:
#
# - TDX_IMX_HAB_CST_CERTS_DIR: Path to directory where certificates would be found.
# - TDX_IMX_HAB_CST_SRK: Path to SRK table file.
# - TDX_IMX_HAB_CST_SRK_FUSE: Path to SRK fuses file.
# - TDX_IMX_HAB_CST_SRK_CERT: Path to SRK certificate file; used with HAB and AHAB.
# - TDX_IMX_HAB_CST_CSF_CERT: Path to CSF certificate file (*); used with HAB only.
# - TDX_IMX_HAB_CST_IMG_CERT: Path to IMG certificate file (*); used with HAB only.
# - TDX_IMX_HAB_CST_SGK_CERT: Path to SGK certificate file (*); used with AHAB only.
#
# (*): Notice that the CSF/IMG/SGK certificates are only ever used when the SRK
#      certificate has the CA flag set.
#
TDX_IMX_HAB_CST_CERTS_DIR ?= "${TDX_IMX_HAB_CST_DIR}/crts"
TDX_IMX_HAB_CST_SRK       ?= "${TDX_IMX_HAB_CST_CERTS_DIR}/SRK_1_2_3_4_table.bin"
TDX_IMX_HAB_CST_SRK_FUSE  ?= "${TDX_IMX_HAB_CST_CERTS_DIR}/SRK_1_2_3_4_fuse.bin"
TDX_IMX_HAB_CST_SRK_CERT  ?= "${@make_srk_cert_name(d, d.getVar('TDX_IMX_HAB_CST_CERTS_DIR'))}"
TDX_IMX_HAB_CST_CSF_CERT  ?= "${@make_sub_cert_name(d, 'CSF', d.getVar('TDX_IMX_HAB_CST_CERTS_DIR'))}"
TDX_IMX_HAB_CST_IMG_CERT  ?= "${@make_sub_cert_name(d, 'IMG', d.getVar('TDX_IMX_HAB_CST_CERTS_DIR'))}"
TDX_IMX_HAB_CST_SGK_CERT  ?= "${@make_sub_cert_name(d, 'SGK', d.getVar('TDX_IMX_HAB_CST_CERTS_DIR'))}"
