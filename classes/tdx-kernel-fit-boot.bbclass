# Make the recipes that install or load the kernel (e.g. image recipes and the
# U-Boot boot script) select the FIT boot flow of the Toradex BSP.
KERNEL_IMAGETYPE:forcevariable = "fitImage"
