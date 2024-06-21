# Data-at-rest encryption

Encryption is the process of encoding information in a format that cannot be read or understood by an eavesdropper, and it might be a required feature for use cases where sensitive data needs to remain confidential and cannot be accessed by unauthorized parties.

Encryption is currently supported on the following SoMs:

- Apalis iMX6
- Apalis iMX8
- Colibri iMX6DL
- Colibri iMX6ULL (1GB eMMC variant only)
- Colibri iMX7D (1GB eMMC variant only)
- Colibri iMX8X
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

Trusted Keys require the availability of a Trust Source for greater security, and CAAM is leveraged on NXP iMX-based SoMs. TPM (Trusted Platform Module) and TEE (Trusted Execution Environment) are also two other possible trust sources for Trusted Keys, but currently not supported by this layer.

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

Here is the complete list of variables that can be used to customize the behavior of this feature:

| Variable | Description | Default value |
| :------- | :---------- | :------------ |
| TDX_ENC_KEY_BACKEND | Backend used to manage the encryption key. Allowed values: `caam` or `cleartext`. If configured with `caam`, it will use Trusted Keys backed by the CAAM device (available on NXP iMX-based SoMs). If configured with `cleartext`, the encryption key will be stored in clear text in the file system (use `cleartext` only for testing purposes!) | `caam` on iMX based SoMs, empty otherwise |
| TDX_ENC_KEY_DIR | Directory to store the encryption key blob | `/var/local/private/.keys` |
| TDX_ENC_KEY_FILE | File name of the encryption key blob | `tdx-enc-key.blob` |
| TDX_ENC_STORAGE_LOCATION | Partition to be encrypted (e.g. `/dev/sdb1`) | Empty |
| TDX_ENC_STORAGE_MOUNTPOINT | Directory to mount the encrypted partition | `/run/encdata` |

IMPORTANT: The script that mounts the encrypted partition runs early in the boot process, where not necessarily udev has run/settled. For that reason, it is recommended to use the name of the partition as assigned by the kernel (e.g. `/dev/sdb1`). If one wants to set a name that relies on udev rules then one must review the systemd dependencies of the service to ensure the name is available.

## Encrypting a partition in the eMMC

In case you want to create an additional partition in the eMMC to store encrypted data, you can use the `tdx-tezi-data-partition` class. For more information, have a look at its documentation ([README-data-partition.md](README-data-partition.md)).
