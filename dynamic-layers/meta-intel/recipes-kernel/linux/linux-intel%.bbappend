FILESEXTRAPATHS:prepend:tdx-signed := "${THISDIR}/linux-intel:"

require ${@ 'classes/include/signed/tdx-signed-uefi-secboot.inc' if 'tdx-signed' in d.getVar('OVERRIDES').split(':') else ''}
require ${@ 'classes/include/signed/tdx-signed-modsign.inc' if 'tdx-signed' in d.getVar('OVERRIDES').split(':') else ''}

MODSIGN_KEY_FILE_CHECKSUMS ?= ""

SRC_URI:append:tdx-signed = " \
    file://composefs.cfg \
    file://lockdown.cfg \
    file://modsign.cfg \
"

do_configure[file-checksums] += "${MODSIGN_KEY_FILE_CHECKSUMS}"

kernel_do_configure:prepend:tdx-signed() {
    modsign_prepare_key
}

INHIBIT_PACKAGE_STRIP:tdx-signed = "1"
INHIBIT_PACKAGE_DEBUG_SPLIT:tdx-signed = "1"
