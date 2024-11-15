# Data-at-rest encryption

Encryption is the process of encoding information in a format that cannot be read or understood by an eavesdropper, and it might be a required feature for use cases where sensitive data needs to remain confidential and cannot be accessed by unauthorized parties.

Encryption is currently supported on the following SoMs:

- Apalis iMX6
- Apalis iMX8
- Colibri iMX6DL
- Colibri iMX6ULL (1GB eMMC variant only)
- Colibri iMX7D (1GB eMMC variant only)
- Colibri iMX8X
- Verdin AM62 (requires the availability of a TPM)
- Verdin iMX8MM
- Verdin iMX8MP

## Data-at-rest encryption implementation

Implementing data-at-rest encryption requires a mechanism to securely store the encryption key in the device, as well as an infrastructure to encrypt and decrypt data on disk at runtime.

For this, the following features are implemented by this layer:

- **Encryption key management**: the Linux kernel Trusted Keys subsystem is used to securely store and manage the private key used to encrypt and decrypt data on the disk.
- **Block device encryption**: the Linux kernel `dm-crypt` subsystem is used to provide transparent disk encryption to users.

## Encryption key management

One of the main challenges to implementing encryption is how to securely store the encryption key. If the key is compromised, the encrypted data can be easily decrypted.

To solve this, a Linux kernel feature called [Trusted Keys](https://docs.kernel.org/security/keys/trusted-encrypted.html) is used.

Trusted Keys make it possible to create and manage variable-length symmetric keys in kernel space, and user space only sees, stores, and loads encrypted blobs.

Trusted Keys require the availability of a Trust Source for greater security. Different Trust Sources are supported, including CAAM (Cryptographic Acceleration and Assurance Module), TPM (Trusted Platform Module) and TEE (Trusted Execution Environment).

This layer supports using CAAM and TPM as a source for managing the encryption key. CAAM is available on NXP iMX-based SoMs and TPM availability might depend on the selected SoM and carrier board.

## Block device encryption

The Linux kernel [dm-crypt](https://docs.kernel.org/admin-guide/device-mapper/dm-crypt.html) subsystem is used to provide transparent disk encryption to users.

`dm-crypt` is part of the device-mapper framework and works by creating a virtual block device that encrypts data as it is written to the disk and decrypts it as it is read.

## Enabling and configuring encryption

To enable the data-at-rest encryption feature, one needs to globally inherit the `tdx-encrypted` class by adding the following line to an OE configuration file (e.g. your `local.conf`):

```
INHERIT += "tdx-encrypted"
```

Also, it is mandatory to set the `TDX_ENC_STORAGE_LOCATION` variable to the disk partition you want to encrypt. For example, to encrypt the `/dev/sdb1` partition:

```
TDX_ENC_STORAGE_LOCATION = "/dev/sdb1"
```

The `TDX_ENC_KEY_BACKEND` variable can be used to configure the trust source for managing the encryption key. If you plan to use CAAM on NXP iMX-based SoMs, you don't have to configure this variable, as it is automatically configured with `caam`. If you plan to use a TPM, you need to configure it as in the example below:

```
TDX_ENC_KEY_BACKEND:forcevariable = "tpm"
```

Make sure to use the `forcevariable` override, so your configuration takes precedence over the default one.

A few additional variables are available to customize the behavior of the data-at-rest encryption feature. Here is list of the most important ones (for the full list refer to `tdx-encrypted.bbclass`):

| Variable | Description | Default value |
| :------- | :---------- | :------------ |
| `TDX_ENC_KEY_BACKEND` | Backend used to manage the encryption key. Allowed values: `caam`, `tpm` or `cleartext`. If configured with `caam`, it will use Trusted Keys backed by the CAAM device (available on NXP iMX-based SoMs). If configured with `tpm`, it will use Trusted Keys backed by a TPM device (availability depends on the hardware). If configured with `cleartext`, the encryption key will be stored in clear text in the file system (use `cleartext` only for testing purposes!) | `caam` on iMX based SoMs, empty otherwise |
| `TDX_ENC_KEY_LOCATION` | Location to store the encryption key blob. Allowed values: `filesystem` or `partition`. If configured with `filesystem`, the encryption key blob will be stored as a file in the filesystem (location defined by the `TDX_ENC_KEY_DIR` variable. If configured with `partition`, the encryption key blob will be stored in a block of the disk outside the dm-crypt partition (useful if the rootfs filesystem is read-only) | `filesystem` |
| `TDX_ENC_KEY_DIR` | Directory to store the encryption key blob | `/var/local/private/.keys` |
| `TDX_ENC_KEY_FILE` | File name of the encryption key blob | `tdx-enc-key.blob` |
| `TDX_ENC_STORAGE_LOCATION` | Partition to be encrypted (e.g. `/dev/sdb1`) | Empty |
| `TDX_ENC_STORAGE_RESERVE` | Number of blocks to reserve from the partition to be encrypted. Each block is 512-byte in size. Might be useful in case one needs a storage location to save data in raw mode, outside the dm-drypt partition. If `TDX_ENC_KEY_LOCATION` is set to `partition`, then the first reserved block is used to store the encryption key blob. | `0` |
| `TDX_ENC_STORAGE_MOUNTPOINT` | Directory to mount the encrypted partition | `/run/encdata` |
| `TDX_ENC_STORAGE_MKFS_ARGS` | Extra arguments to be passed to `mkfs.ext4` when creating the filesystem on the encrypted storage | Empty |
| `TDX_ENC_STORAGE_MOUNT_ARGS` | Extra arguments to be passed to `mount` when mounting the filesystem on the encrypted storage | Empty |

IMPORTANT:

- The service that mounts the encrypted partition (`tdx-enc-handler.service`) runs early in the boot process, where not necessarily udev has run/settled. For that reason, it is recommended to use the name of the partition as assigned by the kernel (e.g. `/dev/sdb1`). If one wants to set a name that relies on udev rules then one must review the systemd dependencies of the service to ensure the name is available.
- Similarly, the location where the encrypted key blob is stored must also be available to the to service; currently the service definition includes dependencies to ensure `/var` is available so that the default configuration works; if storing the key blob outside of `/var` one must review the service definition.

## Notes on using CAAM

When the device is not closed (i.e. secure boot is not enabled), the CAAM backend will use a fixed test key to encrypt the encryption key. That makes it possible to test the encryption feature, but is certainly insecure and not recommended for production usage.

When the device is closed (i.e. secure boot is enabled), the CAAM backend will use the OTPMK key (a never-disclosed 256-bit key randomly generated and fused into each SoC at manufacturing time). This is much more secure and recommended for production use cases.

Be aware that, if you have a device with a partition encrypted with the test key, as soon as you enable secure boot and close the device, you will not be able to read the encrypted partition anymore. This is because CAAM will try to use the OTPMK key to decrypt the encryption key that was previously encrypted with the test key, and that will certainly not work.

To workaround this issue, you can manually remove the key blob, which is by default located at `/var/local/private/.keys/tdx-enc-key.blob`. After removing the key blob and rebooting the device, another key will be generated and the partition will be formatted and encrypted with the new key. As a consequence, you will lose any content on the partition previously encrypted with the test key.

## Notes on using a TPM

Before enabling the TPM backend, you need to make sure there is a TPM device available in the hardware and configured in the operating system. This usually involves enabling the device driver in the Linux kernel and adding a node in the device tree. Be aware that providing access to a TPM device is out of scope in this layer.

To confirm you have a TPM to be used as a Trust Source for managing the encryption key, you can check for the existence of the device node `/dev/tpm0`:

```
# ls -l /dev/tpm0
```

If your hardware lacks a discrete TPM chip, you may want to consider using an fTPM (firmware-based TPM) running in OP-TEE. This layer currently supports OP-TEE and fTPM on Verdin iMX8MP. For additional details, please refer to the [fTPM session in the OP-TEE documentation](README-optee.md#ftpm-support-in-op-tee).

## Encrypting a partition in the eMMC

In case you want to create an additional partition in the eMMC to store encrypted data, you can use the `tdx-tezi-data-partition` class. For more information, have a look at its documentation ([README-data-partition.md](README-data-partition.md)).
