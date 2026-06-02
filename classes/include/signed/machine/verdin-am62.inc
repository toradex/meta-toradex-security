# use tiboot3.bin secure boot variant in Tezi images
FIRMWARE_BINARY[0071] := "tiboot3-am62x-${SYSFW_SUFFIX}-verdin.bin"
FIRMWARE_BINARY[0072] := "tiboot3-am62x-${SYSFW_SUFFIX}-verdin.bin"
FIRMWARE_BINARY[0073] := "tiboot3-am62x-${SYSFW_SUFFIX}-verdin.bin"
FIRMWARE_BINARY[0074] := "tiboot3-am62x-${SYSFW_SUFFIX}-verdin.bin"
FIRMWARE_BINARY[0075] := "tiboot3-am62x-${SYSFW_SUFFIX}-verdin.bin"
FIRMWARE_BINARY[0076] := "tiboot3-am62x-${SYSFW_SUFFIX}-verdin.bin"
FIRMWARE_BINARY[0221] := "tiboot3-am62x-${SYSFW_SUFFIX}-verdin.bin"

# use tiboot3.bin secure boot variant in Tezi recovery images
TEZI_UBOOT_BINARY_RECOVERY:am62xx = "tiboot3-am62x-gp-verdin.bin tiboot3-am62x-${SYSFW_SUFFIX}-verdin.bin u-boot.img-recoverytezi"
