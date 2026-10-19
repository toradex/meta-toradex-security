# use tiboot3.bin secure boot variant in Tezi images
FIRMWARE_BINARY[0088] := "tiboot3-am69-${SYSFW_SUFFIX}-aquila.bin"
FIRMWARE_BINARY[0211] := "tiboot3-am69-${SYSFW_SUFFIX}-aquila.bin"
FIRMWARE_BINARY[0212] := "tiboot3-am69-${SYSFW_SUFFIX}-aquila.bin"
FIRMWARE_BINARY[0213] := "tiboot3-am69-${SYSFW_SUFFIX}-aquila.bin"
FIRMWARE_BINARY[0214] := "tiboot3-am69-${SYSFW_SUFFIX}-aquila.bin"
FIRMWARE_BINARY[0215] := "tiboot3-am69-${SYSFW_SUFFIX}-aquila.bin"
FIRMWARE_BINARY[0223] := "tiboot3-am69-${SYSFW_SUFFIX}-aquila.bin"

# use tiboot3.bin secure boot variant in Tezi recovery images
TEZI_UBOOT_BINARY_RECOVERY:j784s4 = "tiboot3-am69-${SYSFW_SUFFIX}-aquila.bin u-boot.img-recoverytezi"
