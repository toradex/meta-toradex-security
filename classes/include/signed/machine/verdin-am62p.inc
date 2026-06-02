# use tiboot3.bin secure boot variant in Tezi images
FIRMWARE_BINARY[0099] := "tiboot3-am62px-${SYSFW_SUFFIX}-verdin.bin"

# use tiboot3.bin secure boot variant in Tezi recovery images
TEZI_UBOOT_BINARY_RECOVERY:am62pxx = "tiboot3-am62px-${SYSFW_SUFFIX}-verdin.bin u-boot.img-recoverytezi"
