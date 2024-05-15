# OpenEmbedded layer with security-related metadata for Toradex SoMs

This layer contains metadata to enable security features when building a Linux system for Toradex SoMs.

For more information on available Toradex SoMs, please visit:

https://www.toradex.com/computer-on-modules

# Layer dependencies

This layer depends on:

```
URI: git://git.openembedded.org/openembedded-core
layers: meta
branch: kirkstone
revision: HEAD

URI: git://git.yoctoproject.org/meta-security
branch: kirkstone
revision: HEAD

URI: git://git.toradex.com/meta-toradex-nxp.git
branch: kirkstone
revision: HEAD

URI: git://git.toradex.com/meta-toradex-ti.git
branch: kirkstone
revision: HEAD
```

# Supported features and SoMs

This layer supports the following security features:

- Secure boot
  - Bootloader signature checking.
  - U-Boot hardening.
  - FIT image signature checking (kernel, DTBs and ramdisk).
  - Rootfs signature checking via `dm-verity`.

For more information on the level of support in each SoM, please check the correspondent documentation:

| Documentation | Description |
| :------------ | :---------- |
| [docs/README-secure-boot.md](docs/README-secure-boot.md) | General documentation about the secure boot feature |
| [docs/README-secure-boot-imx.md](docs/README-secure-boot-imx.md) | Details on the secure boot implementation for NXP iMX based SoMs |
| [docs/README-secure-boot-k3.md](docs/README-secure-boot-k3.md) | Details on the secure boot implementation for TI K3 based SoMs (e.g. AM62) |

This layer only works on Toradex Embedded Linux 6.3.0 and newer releases.

# License

All metadata is MIT licensed unless otherwise stated. Source code and binaries included in tree for individual recipes are under the LICENSE stated in each recipe (.bb file) unless otherwise stated.

This README document is Copyright (C) 2024 Toradex AG.
