# Data partition

Sometimes it is useful (or even required) to have an additional partition to store application data and other software artifacts:

- If the `tdxref-signed` class is used to enable secure boot, the rootfs partition will be read-only, and an additional partition might be needed to store persistent data.
- If the `tdx-encrypted` class is used to enable encryption, an additional partition in the eMMC can be used to store the encrypted data.

For these and other use cases, this layer supports creating an additional data partition in the eMMC via the `tdx-tezi-data-partition` class.

**Important: This class only works with Toradex Easy Installer images.**

## Enabling the data partition

To enable the creation of the data partition in Toradex Easy Installer images, one needs to globally inherit the `tdx-tezi-data-partition` class by adding the following line to an OE configuration file (e.g. your `local.conf`):

```
INHERIT += "tdx-tezi-data-partition"
```

Additional variables can be used to customize the behavior of this feature:

| Variable | Description | Default value |
| :------- | :---------- | :------------ |
| `TDX_TEZI_DATA_PARTITION_TYPE` | Data partition filesystem type. Supported values are `ext2`, `ext3`, `ext4`, `fat` and `ubifs`. The supported values are limited to what Toradex Easy Installer supports | `ext4` |
| `TDX_TEZI_DATA_PARTITION_LABEL` | Label that will be used to format and mount the data partition | `DATA` |
| `TDX_TEZI_DATA_PARTITION_AUTOMOUNT` | Set to `1` to automatically mount the data partition at boot time, or `0` to disable automouting the partition; when set to `-1` the partition won't even be listed in fstab (it should be mounted by other means) | `-1` if class `tdx-encrypted` is in use or `1` otherwise |
| `TDX_TEZI_DATA_PARTITION_MOUNTPOINT` | Directory where the data partition should be mounted | `/data` |
| `TDX_TEZI_DATA_PARTITION_MOUNT_FLAGS` | Flags used to mount the data partition. See the `mount` man page for more information on the available mount flags | `rw,nosuid,nodev,noatime, errors=remount-ro` |

Additional variables from the `image_type_tezi` class in the `meta-toradex-bsp-common` layer can be used to customize the creation of the data partition. Please see the [source code of this class](https://git.toradex.com/cgit/meta-toradex-bsp-common.git/tree/classes/image_type_tezi.bbclass?h=kirkstone-6.x.y#n37) for more information.

## Encrypting the data partition

The default value of `TDX_TEZI_DATA_PARTITION_AUTOMOUNT` assumes the data partition is going to be encrypted when the class `tdx-encrypted` is also used; this is a common situation but not necessarily true and if not true one must set that variable appropriately for their use case.

When the said assumption is true though, having an encrypted data partition would be achieved by setting (in your `local.conf`, for example):

```
INHERIT += "tdx-tezi-data-partition tdx-encrypted"
TDX_ENC_STORAGE_LOCATION = "/dev/<block-device-for-data-partition>"
```
