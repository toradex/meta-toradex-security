SUMMARY = "Tools for virtual machine firmware volumes"
DESCRIPTION = "This is a small collection of tools for edk2 firmware images.\
They support decoding and printing the content of firmware volumes."
HOMEPAGE = "https://gitlab.com/kraxel/virt-firmware"
LICENSE = "GPL-2.0-only"
LIC_FILES_CHKSUM = "file://LICENSE;md5=fcb02dc552a041dee27e4b85c7396067"

PYPI_PACKAGE = "virt_firmware"

SRC_URI[sha256sum] = "0e23dbae1e8817140ee919344a61b88c533874c360e5a09281d4a82eea0f4e33"

inherit pypi python_setuptools_build_meta

RDEPENDS:${PN} += " \
    python3-core \
    python3-cryptography \
    python3-pefile \
    python3-pyyaml \
"

BBCLASSEXTEND = "native nativesdk"
