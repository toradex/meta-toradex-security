# OpenEmbedded layer with security-related metadata for Toradex SoMs

This layer contains metadata to enable security features when building a Linux system with Toradex SoMs.

For more information on available Toradex SoMs please visit:

https://www.toradex.com/computer-on-modules

# Supported features and SoMs

This layer supports the following security features:

- Secure boot

The level of support of the above features is:

- BSP reference images: tested during development but not in Toradex's CI environment, integration effort is expected.
- Torizon OS (formerly named TorizonCore): tested and integrated.

The features are currently supported on the following SoMs:

- Apalis iMX6
- Apalis iMX8
- Colibri iMX6DL
- Colibri iMX6ULL (1GB eMMC variant only)
- Colibri iMX7D (1GB eMMC variant only)
- Colibri iMX8X
- Verdin AM62
- Verdin iMX8MM
- Verdin iMX8MP

This layer only works on Embedded Linux 6.3.0 and newer releases.

> ⚠️ **_NOTE_**: The main branch is not intended for consumer use. It is not as well maintained as the current yocto branch, and its focus is for our internal CI.

# Secure boot

To enable secure boot, the `tdx-signed` class needs to be inherited in a configuration file.

```
INHERIT += "tdx-signed"
```

When the `tdx-signed` class is inherited, the following features will be enabled (details on the level of support and how to configure might vary depending on the SoM):

- **Bootloader signature checking**: bootloader images are signed at build time and their signature is verified at runtime by the SoC's ROM code.
- **U-Boot hardening**: a few patches are applied to the U-Boot bootloader to make it harder for an attacker to bypass the secure boot process.
- **FIT image signature checking**: a FIT image with the kernel and its artifacts (device trees, ramdisk, etc) is signed at build time and its signature is verified at runtime by the bootloader.

## Bootloader signature checking

The bootloader signature checking implementation is dependent on the System on Chip (SoC).

For details on the bootloader signature checking implementation for SoMs that use NXP iMX-based platforms (i.e. iMX6/7/8), see the `README-imx.md` file.

For details on the bootloader signature checking implementation for SoMs that use TI K3-based platforms (i.e. AM62), see the `README-k3.md` file.

## U-Boot hardening

Toradex is implementing various changes to U-Boot (currently as a series of patches) with the purpose of hardening it for secure boot. The hardening includes the following features:

- **Command whitelisting**: this part of the hardening is responsible for limiting the set of commands available to boot scripts once the device is in closed state - by default, only a small set of commands remain available in that state (mostly those strictly required for booting a secure boot image) alongside a few others considered strictly secure and potentially useful for future boot scripts.
- **Protection against execution of unsigned software by** `bootm`: for securely booting secure boot images the "bootm" command is used in the boot scripts, but this command can also be used insecurely; this part of the hardening tries to ensure only the secure use of the command is possible so that the only possible code path at runtime is that for booting from signed FIT images.
- **CLI access prevention**: this is an extra safeguard whereby the access to the U-Boot CLI gets disabled once the device is in closed state; this is what happens by default (but can be overridden).
- **Kernel command-line protection**: normally U-Boot passes the contents of its environment variable `bootargs` directly to the Linux kernel and this variable in turn has its value set in the persistent U-Boot environment or it is dynamically built by the boot scripts (with Torizon OS following the latter approach) - either way, it is a vector of attack; to prevent tampering of `bootargs` the present protection causes the build to store a copy of the "expected" kernel arguments inside the (signed) FIT image and a related patch to U-Boot to check `bootargs` against that copy at runtime, possibly stopping the boot process in case of a mismatch.

The hardening features above are controlled by the following variables:

| Variable | Description | Default value |
| :------- | :---------- | :------------ |
| `TDX_UBOOT_HARDENING_ENABLE` | Enable hardening features as a whole | `1` if both `TDX_IMX_HAB_ENABLE` and `UBOOT_SIGN_ENABLE` are set or 0 otherwise |
| `TDX_SECBOOT_REQUIRED_BOOTARGS` | Expected value for the fixed part of the kernel command line | Different value for each machine (suitable for Torizon OS) |

Obs.: Currently, U-Boot hardening is not enabled on Verdin AM62.

The behavior of the different hardening features can be set via the control FDT (see [Devicetree Control in U-Boot](https://u-boot.readthedocs.io/en/stable/develop/devicetree/control.html)). Setting the control FDT at build time can be achieved by adding extra device-tree [.dtsi fragments](https://u-boot.readthedocs.io/en/stable/develop/devicetree/control.html#external-dtsi-fragments) to U-Boot and setting the Kconfig variable `CONFIG_DEVICE_TREE_INCLUDES` appropriately; with Yocto/OE this would normally involve adding small patches to U-Boot and appending changes to its recipe but the details are outside the scope of the present document.

The following device-tree fragment shows all the nodes and properties that can be present in the control FDT:

```
/ {
    chosen {
        toradex,secure-boot {          /* if not present: disable Toradex hardening at runtime */
            disabled;                  /* if present: disable Toradex hardening at runtime */
            enable-cli-when-closed;    /* if present: keep u-boot CLI enabled when device is closed */
            bootloader-commands {
                allow-open = <...>;    /* list of command categories allowed when device is open */
                allow-closed = <...>;  /* list of command categories allowed when device is closed */
                deny-open = <...>;     /* list of command categories denied when device is open (use is discouraged) */
                deny-closed = <...>;   /* list of command categories denied when device is closed (use is discouraged) */
                needed = <...>         /* list of command categories strictly needed to boot (use is discouraged) */
            };
        };
    };
};
```

The command categories are currently only available as part of a [patch](./recipes-bsp/u-boot/files/0001-toradex-common-add-command-whitelisting-modules.patch) in header `cmd-categories.h`. The default FDT is part of another [patch](./recipes-bsp/u-boot/files/0002-toradex-dts-add-fragment-file-to-configure-secure-bo.patch) in file `tdx-secboot.dtsi`.

<!-- TODO: Make more user-friendly instructions on setting the control FDT. -->

## Configuring FIT image signing

When the `tdx-signed` class is inherited, generating and signing a FIT image is enabled by default. Set `UBOOT_SIGN_ENABLE` to `0` to disable it.

This feature uses the default FIT image signing support provided by the `uboot-sign` and `kernel-fitimage` classes from OpenEmbedded Core. See the [Yocto Project documentation](https://docs.yoctoproject.org/ref-manual/classes.html#kernel-fitimage) for more details.

A few variables can be used to configure this feature, including:

| Variable | Description | Default value |
| :------- | :---------- | :------------ |
| `UBOOT_SIGN_ENABLE` | Enable signing of FIT image | `1` |
| `FIT_GENERATE_KEYS` | Generate signing keys | `1` |
| `UBOOT_SIGN_KEYDIR` | Location of the RSA key and certificate used for signing | `${TOPDIR}/keys/fit` |
| `UBOOT_SIGN_KEYNAME` | The name of the key used for signing configuration nodes | `dev` |
| `UBOOT_SIGN_IMG_KEYNAME` | The name of the key used for signing individual images | `dev2` |

The complete list of variables can be found in the `tdx-signed-fit-image.inc` file.

# License

All metadata is MIT licensed unless otherwise stated. Source code and binaries included in tree for individual recipes are under the LICENSE stated in each recipe (.bb file) unless otherwise stated.

This README document is Copyright (C) 2023 Toradex AG.
