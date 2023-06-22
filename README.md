# OpenEmbedded layer with security-related metadata for Toradex SoMs

This layer contains metadata to enable security features when building a
Linux system with Toradex SoMs.

For more information on available Toradex SoMs please visit:

https://www.toradex.com/computer-on-modules

# Supported features and SoMs

This layer supports the following security features:

- Secure boot

The features are currently supported on the following SoMs:

- Verdin iMX8MP

# Secure boot

To enable secure boot, the `tdx-signed` class needs to be inherited in a configuration file.

```
INHERIT += "tdx-signed"
```

When secure boot is enabled:

- The bootloader image is signed at build time and its signature is verified at runtime using iMX HAB/AHAB.
- A FIT image with the kernel and its artifacts (device trees, ramdisk, etc) is signed at build time and its signature is verified at runtime by the bootloader.

## Configuring HAB/AHAB support

When the `tdx-signed` class is inherited, signing bootloader images via HAB/AHAB is enabled by default. Set `TDX_IMX_HAB_ENABLE` to `0` to disable it.

Before using this feature, it is required to:

1. Download NXP CST tool from https://www.nxp.com/webapp/sps/download/license.jsp?colCode=IMX_CST_TOOL_NEW.
2. Follow the NXP documentation to generate the keys, certificates, SRK tables and Efuse Hash (the documentation can be found inside the CST tool in `docs/CST_UG.pdf`).

After that, `TDX_IMX_HAB_CST_DIR` and `TDX_IMX_HAB_CST_CERTS_DIR` variables can be used to configure the location of the CST tool and generated certificates. Example:

```
TDX_IMX_HAB_CST_DIR = "/opt/cst"
TDX_IMX_HAB_CST_CERTS_DIR = "/opt/cst/crts"
```

Obs.: The private keys must be located in a directory called `keys` at the same level as the `crts` directory. This is a requirement for the CST tool to work properly.

If HAB/AHAB is enabled, in the end of the build, a file with the commands to fuse the SoC (`fuse-cmds.txt`) will be generated in the images directory. The commands in this file should be executed in the U-Boot command line interface. Read the warning messages carefully and be aware that the commands will write to One-Time Programmable e-fuses, and once you write them, you can't go back! It is recommended to read NXP documentation about HAB/AHAB before writing to the e-fuses. This is an output example of the `fuse-cmds.txt` file:

```
$ cat deploy/images/verdin-imx8mp/fuse-cmds.txt
# These are One-Time Programmable e-fuses. Once you write them you can't
# go back, so get it right the first time!
fuse prog -y 6 0 0x8AE322B2
fuse prog -y 6 1 0xDF2939A3
fuse prog -y 6 2 0x9DA80323
fuse prog -y 6 3 0x3B024EF2
fuse prog -y 7 0 0xA53091
fuse prog -y 7 1 0x55304E7A
fuse prog -y 7 2 0xFB8FF259
fuse prog -y 7 3 0x9CE57582

# After the device successfully boots a signed image without generating
# any HAB events, it is safe to secure, or 'close', the device. This is
# the last step in the process. Once the fuse is blown, the chip does
# not load an image that has not been signed using the correct PKI tree.
# Be careful! This is again a One-Time Programmable e-fuse. Once you
# write it you can't go back, so get it right the first time. If
# anything in the previous steps wasn't done correctly, after writing
# this bit, the SOM will not boot anymore!
fuse prog -y 1 3 0x02000000
```

Summary of the variables that can be used to configure HAB/AHAB support:

| Variable | Description | Default value |
| :------- | :---------- | :------------ |
| `TDX_IMX_HAB_ENABLE` | Enable/disable HAB/AHAB support | `1` |
| `TDX_IMX_HAB_CST_KEY_SIZE` | Size of the generated keys | `2048` |
| `TDX_IMX_HAB_CST_DIR` | Location of the CST tool | `${TOPDIR}/keys/cst` |
| `TDX_IMX_HAB_CST_CERTS_DIR` | Location of the certificates directory | `${TDX_IMX_HAB_CST_DIR}/crts` |

The complete list of variables can be found in the `imx-hab.bbclass` file.

## Configuring FIT image signing

When the `tdx-signed` class is inherited, generating and signing a FIT image is enabled by default. Set `UBOOT_SIGN_ENABLE` to `0` to disable it.

This features uses the default FIT image signing support provided by the `uboot-sign` and `kernel-fitimage` classes from OpenEmbedded Core. See the [Yocto Project documentation](https://docs.yoctoproject.org/ref-manual/classes.html#kernel-fitimage) for more details.

A few variables can be used to configure this feature, including:

| Variable | Description | Default value |
| :------- | :---------- | :------------ |
| UBOOT_SIGN_ENABLE | Enable signing of FIT image | `1` |
| FIT_GENERATE_KEYS | Generate signing keys | `1` |
| UBOOT_SIGN_KEYDIR | Location of the RSA key and certificate used for signing | `${TOPDIR}/keys/fit` |
| UBOOT_SIGN_KEYNAME | The name of the key used for signing | `dev` |

The complete list of variables can be found in the `tdx-signed-fit-image.inc` file.

# License

All metadata is MIT licensed unless otherwise stated. Source code and
binaries included in tree for individual recipes is under the LICENSE
stated in each recipe (.bb file) unless otherwise stated.

This README document is Copyright (C) 2023 Toradex AG.
