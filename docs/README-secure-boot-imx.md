# Bootloader signature checking for NXP iMX-based platforms

This document describes how bootloader signature checking works for System on Modules (SoMs) that use iMX-based System on Chips (SoCs) by NXP Semiconductors.

## Introduction

To support bootloader signature checking, such a feature needs to be available in the SoC ROM code.

On iMX6, iMX7 and iMX8M, this feature is available and it is called HAB (High Assurance Boot).

On iMX8, iMX8X and iMX95, this feature is available and it is called AHAB (Advanced High Assurance Boot).

## Configuring HAB/AHAB support

When the `tdx-signed` class is inherited, signing bootloader images via HAB/AHAB is enabled by default. Set `TDX_IMX_HAB_ENABLE` to `0` to disable it.

Before using this feature, it is required to:

1. Download NXP CST tool from https://www.nxp.com/webapp/sps/download/license.jsp?colCode=IMX_CST_TOOL_NEW.
2. Follow the NXP documentation to generate the keys, certificates, SRK tables and Efuse Hash (the documentation can be found inside the CST tool in `docs/CST_UG.pdf`); be sure to take note of your answers to the key generation script.

After that, configure the various variables listed below to match your choices; pay special attention to the ones depending on your answers to the NXP key generation script.

| Variable | Description | Default value |
| :------- | :---------- | :------------ |
| `TDX_IMX_HAB_ENABLE` | Enable/disable HAB/AHAB support; allowed values: `0` or `1`. | `1` |
| `TDX_IMX_HAB_CST_DIR` | Location of the CST tool. | `${TOPDIR}/keys/cst` |
| `TDX_IMX_HAB_CST_BIN` | Name of the CST binary tool. | `${TDX_IMX_HAB_CST_DIR}/linux64/bin/cst` |
| `TDX_IMX_HAB_CST_ARGS` | Additional parameters to be passed to the CST tool | Empty |
| `TDX_IMX_HAB_CST_CERTS_DIR` | Location of the certificates directory. The associated private keys must be located in a directory called `keys` at the same level as the `crts` directory (this is a requirement for the CST tool to work properly). | `${TDX_IMX_HAB_CST_DIR}/crts` |
| `TDX_IMX_HAB_CST_CRYPTO` | Type of cryptographic keys in use; allowed values: `rsa` or `ecdsa`. This should be set to `ecdsa` if (and only if) you selected "Elliptic Curve Cryptography" when generating the keys/certificates with the CST tool. | `rsa` |
| `TDX_IMX_HAB_CST_KEY_SIZE` | For **RSA** keys, this would be the key length (in bits) as entered into the CST tool. For **ECDSA**, this would be a string determined from the generated certificate file name; for example, for a file named `SRK1_sha256_secp384r1_v3_ca_crt.pem` (found in the certificates directory) the present variable would be set to `secp384r1`. | `2048` |
| `TDX_IMX_HAB_CST_KEY_EXP` | Key exponent for RSA keys (only). | `65537` |
| `TDX_IMX_HAB_CST_DIG_ALGO` | Digest algorithm as entered into the CST tool. | `sha256` |
| `TDX_IMX_HAB_CST_SRK_CA` | Whether or not the SRK certificates have the CA flag set as entered into the CST tool; allowed values: `0` or `1`. | `1` |
| `TDX_IMX_HAB_CST_SRK_INDEX` | Index of the SRK to be used for signing within the SRK table; allowed values: `1`..`4`, corresponding to `SRK1`..`SRK4`, respectively. | `1` |
| `TDX_IMX_HAB_GEN_UBOOT_FUSING_CMD` | Add a function called `prog_secure_boot_fuses` to U-Boot's environment to program the secure boot fuses. Since this is intended mostly for development/tests, the function will only program the fuses but not close the device. If you enable it and run the function inside U-Boot, be aware that it will write to One-Time Programmable e-fuses, and the operation is irreversible! Allowed values are: `0` (disabled) or `1` (enabled). | `0` |

The complete list of variables can be found in the `imx-hab.bbclass` file.

**NOTE**: For HAB signing, [libfaketime](https://github.com/wolfcw/libfaketime) is used when generating the CSF binaries with CST in order to create reproducible bootloader image builds.

### Known issues

- Starting from version 4.0.0, the NXP CST tool may not work as expected on older Linux distributions (e.g., Ubuntu 20.04). If you are using NXP CST 4.0.0 or later, it is recommended to use a more recent Linux distribution (e.g., Ubuntu 24.04) to ensure compatibility and avoid potential issues.
- As of April 2025, SGK is not supported in the current firmware for SoCs using the EdgeLock Secure Enclave (ELE), which includes iMX8ULP and iMX9x. Consequently, when building for machines based on these SoCs, the use of subordinate signing keys (SGK) will be disabled, and the boot container will be signed directly with the Super Root Keys (SRK).
- On i.MX95 silicon revisions A0 and A1, the fuse command in U-Boot does not work with the latest Toradex BSP. If you are using one of these early silicon revisions and encounter issues when programming fuses, try using an older Toradex Easy Installer release (prior to August). If the problem persists, please open an issue in this repository or contact the Toradex support team for assistance.

### Closing the device

If HAB/AHAB is enabled, at the end of the build, a file with the commands to fuse the SoC (`fuse-cmds.txt`) will be generated in the images directory. The commands in this file should be executed in the U-Boot command line interface.

Read the warning messages carefully and be aware that the commands will write to One-Time Programmable e-fuses, and once you write them, you can't go back! You can check for HAB events with the command `hab_status` for HAB or `ahab_status` for AHAB. It is recommended to read NXP documentation about HAB/AHAB before writing to the e-fuses. This is an output example of the `fuse-cmds.txt` file:

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

Alongside `fuse-cmds.txt`, the build also generates `imx-config.fuse`. This file contains the same fusing configuration, but in a machine-readable format, intended for programmatic consumption by other tools or recipes.

```
$ cat deploy/images/verdin-imx8mp/imx-config.fuse
H:T:HAB
H:F:6:0:0xB9BB8A0C
H:F:6:1:0x2FF6C619
H:F:6:2:0x79B3A9F0
H:F:6:3:0x9D426FE6
H:F:7:0:0x92523418
H:F:7:1:0xD01D4E2B
H:F:7:2:0xA23CCF8C
H:F:7:3:0x3D794BAC
H:C:1:3:0x02000000
```
