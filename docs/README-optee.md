# OP-TEE

A Trusted Execution Environment (TEE) is an environment where the code executed and the data accessed are isolated and protected in terms of confidentiality (no one has access to the data) and integrity (no one can change the code and its behavior).

TEE might be a good solution to store and manage secrets (e.g. encryption keys) and isolate the execution of security-sensitive operations like biometric authentication, digital payments, copyright protection, etc.

[OP-TEE](https://www.trustedfirmware.org/projects/op-tee/) (Open Portable Trusted Execution Environment) is an open-source TEE designed as a companion to a non-secure Linux kernel running on ARM Cortex-A cores using the TrustZone technology.

This layer provides support for running OP-TEE on the following SoMs:

- Verdin iMX8MP

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

## Read-write filesystem for OP-TEE

OP-TEE needs a read-write filesystem (by default, it writes to `/data/tee/`). When rootfs signature checking is enabled via the `tdxref-signed` class, the rootfs image will be generated using the `dm-verity` kernel feature, which is read-only.

In this case, to provide a read-write filesystem to OP-TEE, the `tdx-tezi-data-partition` class will be automatically inherited. This class will create an additional partition in the eMMC and mount it by default at `/data`. For more information on how this class works, have a look at its documentation ([README-data-partition.md](README-data-partition.md)).

In case your rootfs is read-only and you are not using a Toradex Easy Installer image, make sure the `/data` directory is mounted at a read-write partition, or OP-TEE will not work properly.

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
