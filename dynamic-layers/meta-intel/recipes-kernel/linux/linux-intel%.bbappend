FILESEXTRAPATHS:prepend:tdx-signed := "${THISDIR}/linux-intel:"

require ${@ 'classes/include/signed/tdx-signed-uefi-secboot.inc' if 'tdx-signed' in d.getVar('OVERRIDES').split(':') else ''}

SRC_URI:append:tdx-signed = " \
    file://lockdown.cfg \
    file://modsign.cfg \
"
