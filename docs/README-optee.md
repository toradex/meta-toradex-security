# OP-TEE

A Trusted Execution Environment (TEE) is an environment where the code executed and the data accessed are isolated and protected in terms of confidentiality (no one has access to the data) and integrity (no one can change the code and its behavior).

TEE might be a good solution to store and manage secrets (e.g. encryption keys) and isolate the execution of security-sensitive operations like biometric authentication, digital payments, copyright protection, etc.

[OP-TEE](https://www.trustedfirmware.org/projects/op-tee/) (Open Portable Trusted Execution Environment) is an open-source TEE designed as a companion to a non-secure Linux kernel running on ARM Cortex-A cores using the TrustZone technology.

This layer provides support for running OP-TEE on the following SoMs:

- Verdin iMX8MP
- Verdin iMX8MM
- Verdin AM62

## Enabling OP-TEE

To enable OP-TEE on the supported hardware platforms, one needs to globally inherit the `tdx-optee` class by adding the following line to an OE configuration file (e.g. `local.conf`):

```
INHERIT += "tdx-optee"
```

A few variables can be used to customize the behavior of this feature:

| Variable | Description | Default value |
| :------- | :---------- | :------------ |
| TDX_OPTEE_DEBUG | Enable OP-TEE debug messages to the serial console (`1` to enable or `0` to disable) | `0` |
| TDX_OPTEE_INSTALL_TESTS | Enable the installation of OP-TEE test applications (`1` to enable or `0` to disable) | `0` |
| TDX_OPTEE_FS_RPMB | Enable support for using the eMMC RPMB partition as a secure storage device (`1` to enable or `0` to disable) | `0` |
| TDX_OPTEE_FS_RPMB_DEV_ID | Configure the eMMC RPMB partition device node. For example, if configured with `2`, the TEE supplicant will use `/dev/mmcblk2rpmb` to communicate with to the RPMB partition | `0` |
| TDX_OPTEE_FS_RPMB_MODE | RPMB secure storage operation mode. Valid options are `development` (to be used during development), `factory` (to enroll the RPMB key in a secure provisioning environment) and `production` (to be used in production). For more information on how to configure this variable, see the session [RPMB support in OP-TEE](/docs/README-optee.md#rpmb-support-in-op-tee) | `development` |
| TDX_OPTEE_FTPM | Enable support for a firmware TPM (fTPM) implementation running as trusted application in OP-TEE (`1` to enable or `0` to disable). For more information on how an fTPM works, see the session [fTPM support in OP-TEE](/docs/README-optee.md#ftpm-support-in-op-tee) | `0` |

## Testing OP-TEE

OP-TEE can be validated using its test applications, which can be installed in the image by adding the following line to an OE configuration file (e.g. `local.conf`):

```
TDX_OPTEE_INSTALL_TESTS += "1"
```

After building with the test applications, a "hello world" application can be used to test the communication between the Normal World (Linux) and the Secure World (OP-TEE):

```
# optee_example_hello_world
```

There is also a complete test suite that can be used to validate OP-TEE:

```
# xtest
```

In case of issues, try enabling debug messages via the `TDX_OPTEE_DEBUG` variable.

## Secure storage for persistent data in OP-TEE

OP-TEE requires a secure storage solution to save persistent data, including cryptographic keys, key materials, and general-purpose data.

There are currently two secure storage implementations in OP-TEE:

- The first one relies on the normal world (REE) file system.
- The second one makes use of the Replay Protected Memory Block (RPMB) partition of an eMMC device.

When relying on the normal world file system, OP-TEE uses `/data/tee/` as its secure storage location in the Linux file system. All stored data is encrypted using a Hardware Unique Key (HUK), which is supplied by the CAAM driver on i.MX platforms and the DMSC subsystem on K3-based devices (e.g., AM6X).

To be able to write and persist data, OP-TEE needs a read-write filesystem mounted at `/data`. When rootfs signature checking is enabled via the `tdxref-signed` class, the rootfs image will be generated using the `dm-verity` kernel feature, which is read-only. In this case, to provide a read-write filesystem to OP-TEE, the `tdx-tezi-data-partition` class will be automatically inherited. This class will create an additional partition in the eMMC and mount it by default at `/data`. For more information on how this class works, have a look at its documentation ([README-data-partition.md](README-data-partition.md)).

If your rootfs is read-only and you are not using a Toradex Easy Installer image, ensure that the `/data` directory is mounted on a writable partition. Failing to do so will prevent OP-TEE from functioning properly (in case you are relying on the normal world file system for secure storage).

When using RPMB (Replay Protected Memory Block) as the secure storage device, a writable `/data` partition is not required. The RPMB partition is used instead for secure storage. For more information on RPMB usage with OP-TEE, see the section [RPMB support in OP-TEE](/docs/README-optee.md#rpmb-support-in-op-tee).

For further details on how secure storage works in OP-TEE, refer to the [OP-TEE documentation](https://optee.readthedocs.io/en/latest/architecture/secure_storage.html).

## RPMB support in OP-TEE

RPMB (Replay Protected Memory Block) is a dedicated partition available on some flash-based storage devices (eMMC, UFS, NVMe, etc) that makes it possible to store and retrieve data with integrity and authenticity support. It was introduced in eMMC version 4.4, and it is available and ready to be used on most Toradex modules.

When using the RPMB partition, a symmetric key is used to authenticate reads and writes. In a nutshell, here is how it works:

- An authentication key is first programmed to the storage device. This step must occur in a secure environment, typically in a secure factory setup.
- When writing to the device, the data is hashed and signed with the authentication key, and the storage device will only accept the write operation after checking the signature (this signature is also called message authentication code, or just MAC).
- When reading from the device, the data is returned together with the MAC. The host can also calculate the MAC and compare it with the one received to make sure the message is authentic.

OP-TEE can use the RPMB partition as a secure storage solution instead of relying on the Linux filesystem. To enable this feature, set the `TDX_OPTEE_FS_RPMB` variable to `1`.

One significant challenge with RPMB is the requirement to write a private key to the partition before any operations. This key programming can only be done once and must occur in a secure provisioning environment.

To address this, the `TDX_OPTEE_FS_RPMB_MODE` variable is introduced, allowing you to configure the RPMB operation mode. The supported values are: `development`, `factory`, and `production`:

- `development`: The RPMB partition is emulated in software by `tee-supplicant`. Be aware that the emulation is done in memory, so the contents of the storage is lost after a reboot or when the `tee-supplicant` daemon is restarted. Suitable option for development.
- `factory`: The eMMC RPMB partition is used, and OP-TEE is configured to program the RPMB key, derived from the SoC's Hardware Unique Key (HUK) and other device specific information. This mode is intended for secure factory provisioning environments, and it should never be used outside of a trusted, secure factory setup.
- `production`: The eMMC RPMB partition is used with the assumption that the RPMB key has already been programmed (enrolled). Designed for production environments where secure storage relies on a pre-provisioned RPMB key.

With this, one could use an OS image configured in `factory` mode during a secure provisioning process, then deploy an OS image configured in `production` mode to devices in the field. However, be aware that alternative RPMB key provisioning methods exist, and the approach should align with your threat model.

Another important aspect is how the key generation works. Because OP-TEE derives the RPMB authentication key from the SoC's HUK, if the RPMB key is provisioned while the device is in an "open" state (secure boot disabled), it becomes inaccessible once secure boot is enabled, because the HUK changes when the device transitions to a "closed" state.

To prevent this issue, OP-TEE includes a software hook, `plat_rpmb_key_is_ready()`, which platforms can use to enforce a security logic. For example, on i.MX-based devices, the RPMB key cannot be written unless secure boot is enabled. This behavior can be modified by overriding the `plat_rpmb_key_is_ready()` function in the `core/drivers/imx_snvs.c` file of the OP-TEE OS source code.

To test RPMB functionality, use OP-TEE's test tools to write to the storage and verify if the RPMB write counter increases. Note that this test will not work in development mode since the RPMB partition is emulated in memory.

```
# mmc rpmb read-counter /dev/mmcblk*rpmb
# optee_example_secure_storage
# mmc rpmb read-counter /dev/mmcblk*rpmb
```

## fTPM support in OP-TEE

Trusted Platform Modules (TPMs) are designed to enhance system security by performing cryptographic operations such as key generation, encryption, and digital signing. Their primary role is to provide secure storage for cryptographic keys and ensure system integrity by measuring the software environment during the boot process.

An fTPM (Firmware-based Trusted Platform Module) is a type of TPM that operates within firmware, as opposed to being a discrete hardware chip. While it offers similar functionality, it runs inside a Trusted Execution Environment (TEE), such as OP-TEE.

Using this layer, fTPM can be enabled by adding the following line to an OE configuration file (e.g. `local.conf`):

```
TDX_OPTEE_FTPM = "1"
```

When fTPM is enabled, you should have a TPM device at `/dev`:

```
# ls -l /dev/tpm*
```

And the TPM2 tools can be used to test it:

```
# tpm2_getrandom 32 | hexdump -C
```

It's important to note that an fTPM may not entirely replace the need for a discrete TPM chip, as this depends on the specific use case and the product's threat model. Nevertheless, an fTPM can provide adequate security for certain scenarios, especially when incorporating a hardware TPM chip is impractical.

## Important note

This layer provides the foundational infrastructure to enable OP-TEE functionality on supported hardware platforms. However, it is important to note that this layer is not designed to serve as a comprehensive, ready-to-deploy security solution for all use cases. Always make sure to review the implementation and adapt it to your own needs.
