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
- Data-at-rest encryption
  - Encryption key management via the Trusted Keys kernel subsystem
  - Block device encryption with `dm-crypt`
- TEE (Trusted Execution Environment)
  - Support for running OP-TEE

For more information on the available features, please check the corresponding documentation:

| Documentation | Description |
| :------------ | :---------- |
| [docs/README-secure-boot.md](docs/README-secure-boot.md) | General documentation about the secure boot feature |
| [docs/README-secure-boot-imx.md](docs/README-secure-boot-imx.md) | Details on the secure boot implementation for NXP iMX based SoMs |
| [docs/README-secure-boot-k3.md](docs/README-secure-boot-k3.md) | Details on the secure boot implementation for TI K3 based SoMs (e.g. AM62) |
| [docs/README-encryption.md](docs/README-encryption.md) | General documentation about the data-at-rest encryption feature |
| [docs/README-optee.md](docs/README-optee.md) | Documentation on how to run a Trusted Execution Environment (OP-TEE) together with the Linux kernel |
| [docs/README-data-partition.md](docs/README-data-partition.md) | Documentation on how to create an additional partition for storing persistent data |

This layer only works on Toradex Embedded Linux BSP 6.3.0 and newer releases.

# License

All metadata is MIT licensed unless otherwise stated. Source code and binaries included in tree for individual recipes are under the LICENSE stated in each recipe (.bb file) unless otherwise stated.

This README document is Copyright (C) 2024 Toradex AG.
