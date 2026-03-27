# HSM-backed image signing

This document describes how to use a PKCS#11-compatible HSM to sign secure-boot artifacts with `meta-toradex-security`.

With this workflow, private signing keys do not need to be stored on the build host filesystem. Instead, signing operations are delegated through a PKCS#11 provider to an HSM-backed device or service, such as a hardware token or a CloudHSM offering.

The current implementation supports HSM-backed signing for the complete secure boot flow on i.MX-based SoMs. Support for TI-based SoMs is still a work in progress.

## How to use

### Prerequisites

Before enabling this feature, make sure you have:

- a PKCS#11-compatible token or HSM provisioned with the signing keys
- a PKCS#11 provider library for that token or HSM
- a native OpenEmbedded recipe that makes the PKCS#11 provider available in the native sysroot
- the token PIN
- the token URI and object labels for the signing keys

> **Note:** Provisioning the HSM or token with the signing keys is currently outside the scope of this document.

### Enabling HSM-backed image signing

HSM-backed image signing can be enabled by setting:

```
TDX_SIGNED_HSM = "1"
```

When enabled, the build system uses the HSM through PKCS#11 to sign the images instead of relying on private keys stored on the filesystem. The variables described below can then be used to configure this behavior.

### Configuration variables

The following variables are available to configure HSM-backed signing of secure boot images:

| Variable | Description | Default value |
| :------- | :---------- | :------------ |
| `TDX_SIGNED_HSM` | Enable HSM-backed image signing. Allowed values are `0` and `1`. | `0` |
| `TDX_SIGNED_HSM_TOKEN_PIN` | PIN used to access the token during signing. Avoid hardcoding this value in configuration files. It is recommended to provide it through an environment variable instead. See the following sections of this document for more details. | `""` |
| `TDX_SIGNED_HSM_PKCS11_MODULE_PROVIDER` | Native package that provides the PKCS#11 implementation for the HSM. It is added as a dependency to recipes that need to access the token during signing. | `""` |
| `TDX_SIGNED_HSM_PKCS11_MODULE_PATH` | Path to the PKCS#11 shared library, relative to the recipe native sysroot. | `""` |
| `TDX_SIGNED_HSM_PKCS11_SOFTHSM_CONF` | Path to the SoftHSM configuration file. This is required when using SoftHSM, a software implementation of a PKCS#11 token commonly used for development and testing. | `${TOPDIR}/keys/softhsm/conf/softhsm2.conf` |
| `TDX_SIGNED_HSM_FIT_TOKEN_URL` | PKCS#11 URI identifying the token that contains the FIT signing key. | `""` |
| `TDX_SIGNED_HSM_FIT_TOKEN_LABEL` | Label of the private key object used to sign the FIT image. | `""` |
| `TDX_SIGNED_HSM_SUPPRESS_WARNINGS` | Suppress sanity-check warnings emitted when HSM variables are missing. Allowed values are `0` and `1`. | `0` |

The following sections provide more details on how these variables are used.

### Configuring the PKCS#11 provider

A PKCS#11 provider is the shared library that implements the PKCS#11 API for a given HSM or token. It is the component that allows the signing tools used by the build system to communicate with the HSM and perform private key operations using the keys stored in the device.

Configuring the PKCS#11 provider is one of the first steps when enabling HSM-backed image signing.

The most important variables for configuring the PKCS#11 provider are `TDX_SIGNED_HSM_PKCS11_MODULE_PROVIDER` and `TDX_SIGNED_HSM_PKCS11_MODULE_PATH`. The first ensures that the PKCS#11 module is built and installed into the native sysroot, and the second tells the signing tools which shared library to load.

The HSM signing flow in `meta-toradex-security` is designed to be provider-agnostic, as long as the HSM is accessible through a PKCS#11-compatible shared library available in the native sysroot. At the moment, the layer has been validated with the following providers:

- **SoftHSM**, a software implementation of a PKCS#11 token.
- **YubiKey**, a hardware-backed signing device from [Yubico](https://www.yubico.com/).

SoftHSM is useful for development, testing, and CI because it provides a PKCS#11-compatible software token without requiring dedicated hardware. Example configuration:

```
TDX_SIGNED_HSM_PKCS11_MODULE_PROVIDER = "softhsm-native"
TDX_SIGNED_HSM_PKCS11_MODULE_PATH = "/usr/lib/softhsm/libsofthsm2.so"
TDX_SIGNED_HSM_PKCS11_SOFTHSM_CONF = "${TOPDIR}/keys/softhsm/conf/softhsm2.conf"
```

YubiKey is a widely used hardware-backed signing device and can be enabled with the following configuration:

```
TDX_SIGNED_HSM_PKCS11_MODULE_PROVIDER = "yubico-piv-tool-native"
TDX_SIGNED_HSM_PKCS11_MODULE_PATH = "/usr/lib/libykcs11.so"
```

To use a different PKCS#11 provider, the layer does not usually require code changes, as long as the provider can be made available in the native sysroot and is compatible with the signing tools. In general, the following steps are required:

- Make sure the HSM or token exposes a standard PKCS#11 shared library.
- Add or reuse a native OpenEmbedded recipe that installs that shared library into the native sysroot.
- Set `TDX_SIGNED_HSM_PKCS11_MODULE_PROVIDER` to the native recipe that provides the PKCS#11 module.
- Set `TDX_SIGNED_HSM_PKCS11_MODULE_PATH` to the path of the shared library inside the native sysroot.

> **Note:** If the provider requires additional environment variables or runtime initialization, the build logic may need to be extended accordingly. For a reference, see `classes/tdx-signed-hsm-env.bbclass`.

### Configuring HAB/AHAB signing (only for NXP iMX based SoMs)

When signing boot images for NXP i.MX platforms with HAB or AHAB enabled, CST normally expects the signing certificates to be provided as files on the build host filesystem.

In a traditional filesystem-based setup, variables such as `TDX_IMX_HAB_CST_SRK_CERT`, `TDX_IMX_HAB_CST_CSF_CERT`, `TDX_IMX_HAB_CST_IMG_CERT`, and `TDX_IMX_HAB_CST_SGK_CERT` point to certificate files stored locally. CST then uses those files as inputs during the signing process (see [README-secure-boot-imx.md](README-secure-boot-imx.md) for more information).

With HSM-backed signing, the goal is to avoid depending on locally stored key material and instead let CST access the required objects through the PKCS#11 interface. For this reason, these variables are configured with **PKCS#11 URIs** instead of regular filesystem paths.

A PKCS#11 URI identifies an object inside the token, such as a certificate, using attributes like the token name, object ID, and object type. By replacing the certificate filename with a PKCS#11 URI, CST can resolve the certificate directly from the HSM or token instead of reading it from the filesystem.

This is the key idea behind the HAB/AHAB HSM integration: the signing flow keeps the same CST variables and the same general signing process, but the values of those variables now refer to objects inside the HSM rather than files in a local directory.

For example, in a HAB configuration the certificate variables may be set as follows:

```
TDX_IMX_HAB_CST_SRK_CERT = "pkcs11:token=cst-hsm;object=SRK1_sha256_2048_65537_v3_ca;type=cert;pin-value=${TDX_SIGNED_HSM_TOKEN_PIN}"
TDX_IMX_HAB_CST_CSF_CERT = "pkcs11:token=cst-hsm;object=CSF1_1_sha256_2048_65537_v3_usr;type=cert;pin-value=${TDX_SIGNED_HSM_TOKEN_PIN}"
TDX_IMX_HAB_CST_IMG_CERT = "pkcs11:token=cst-hsm;object=IMG1_1_sha256_2048_65537_v3_usr;type=cert;pin-value=${TDX_SIGNED_HSM_TOKEN_PIN}"
```

In an AHAB configuration, the corresponding variables are typically:

```
TDX_IMX_HAB_CST_SRK_CERT = "pkcs11:token=cst-hsm;object=.%2FSRK1_sha256_2048_65537_v3_ca;type=cert;pin-value=${TDX_SIGNED_HSM_TOKEN_PIN}"
TDX_IMX_HAB_CST_SGK_CERT = "pkcs11:token=cst-hsm;object=.%2FSGK1_1_sha256_2048_65537_v3_usr;type=cert;pin-value=${TDX_SIGNED_HSM_TOKEN_PIN}"
```

In these examples:

- `token=...` identifies the PKCS#11 token
- `object=...` identifies the certificate object inside the token
- `type=cert` indicates that the referenced object is a certificate
- `pin-value=...` provides the token PIN used to access the object

This configuration changes how CST locates the certificate objects. It does not change the role of each variable in the HAB/AHAB signing flow. The same variables still represent the SRK, CSF, IMG, or SGK certificates as in the filesystem-based setup; only the way they are referenced is different.

> **Note:** When HSM-backed signing is enabled for NXP i.MX secure-boot builds, the CST tool is built from source so that PKCS#11 support is available.

### Configuring FIT image signing

FIT image signing is handled by U-Boot's `mkimage` tool, which signs the FIT image using a private key. In the default filesystem-based setup, this key is expected to be available as a file on the build host. When HSM-backed signing is enabled, the private key is no longer read from the filesystem. Instead, the signing tool accesses it through the PKCS#11 interface.

The main idea is similar to the HAB/AHAB configuration described in the previous section: instead of referring to local key files, the build is configured to reference key material stored in the HSM.

In this case, however, the configuration is split into two variables: `TDX_SIGNED_HSM_FIT_TOKEN_URL` identifies the **token** and `TDX_SIGNED_HSM_FIT_TOKEN_LABEL` defines the **private key object inside that token**. The build system uses them to configure the U-Boot signing flow accordingly.

The example below demonstrates the use of these variables:

```conf
TDX_SIGNED_HSM_FIT_TOKEN_URL = "model=SoftHSM%20v2;manufacturer=SoftHSM%20project;serial=984ed1d2002d1c09;token=fit-hsm"
TDX_SIGNED_HSM_FIT_TOKEN_LABEL = "fit-sign-key"
```

> **Note:** The exact token URL format depends on the PKCS#11 provider and the token being used. Attributes such as `model`, `manufacturer`, `serial`, and `token` may vary depending on the device and on how the token was provisioned.

### Final configuration examples

The following example shows a development setup using SoftHSM for image signing on iMX HAB based SoMs:

```
TDX_SIGNED_HSM = "1"

TDX_SIGNED_HSM_PKCS11_MODULE_PROVIDER = "softhsm-native"
TDX_SIGNED_HSM_PKCS11_MODULE_PATH = "/usr/lib/softhsm/libsofthsm2.so"

TDX_IMX_HAB_CST_SRK_CERT = "pkcs11:token=cst-hsm;object=SRK1_sha256_2048_65537_v3_ca;type=cert;pin-value=${TDX_SIGNED_HSM_TOKEN_PIN}"
TDX_IMX_HAB_CST_CSF_CERT = "pkcs11:token=cst-hsm;object=CSF1_1_sha256_2048_65537_v3_usr;type=cert;pin-value=${TDX_SIGNED_HSM_TOKEN_PIN}"
TDX_IMX_HAB_CST_IMG_CERT = "pkcs11:token=cst-hsm;object=IMG1_1_sha256_2048_65537_v3_usr;type=cert;pin-value=${TDX_SIGNED_HSM_TOKEN_PIN}"

TDX_SIGNED_HSM_FIT_TOKEN_URL = "model=SoftHSM%20v2;manufacturer=SoftHSM%20project;serial=984ed1d2002d1c09;token=fit-hsm"
TDX_SIGNED_HSM_FIT_TOKEN_LABEL = "fit-sign-key"
```

And the following example shows the configuration using Yubikey for image signing on iMX AHAB based SoMs:

```
TDX_SIGNED_HSM = "1"

TDX_SIGNED_HSM_PKCS11_MODULE_PROVIDER = "yubico-piv-tool-native"
TDX_SIGNED_HSM_PKCS11_MODULE_PATH = "/usr/lib/libykcs11.so"

TDX_IMX_HAB_CST_SRK_CERT = "pkcs11:token=YubiKey%20PIV%20%2322863375;id=%06;type=cert;pin-value=${TDX_SIGNED_HSM_TOKEN_PIN}" 
TDX_IMX_HAB_CST_SGK_CERT = "pkcs11:token=YubiKey%20PIV%20%2322863375;id=%07;type=cert;pin-value=${TDX_SIGNED_HSM_TOKEN_PIN}"

TDX_SIGNED_HSM_FIT_TOKEN_URL = "model=YubiKey%20YK5;manufacturer=Yubico%20%28www.yubico.com%29;serial=22863375;token=YubiKey%20PIV%20%2322863375;id=%12"
TDX_SIGNED_HSM_FIT_TOKEN_LABEL = "yk-dev-key"
```

### Building the image

When generating an image signed by HSM-backed keys, a critical step is providing the token PIN to the build system.

Although `TDX_SIGNED_HSM_TOKEN_PIN` can be set like any other configuration variable, it is **not recommended** to hardcode the PIN in configuration files such as `local.conf`, machine configuration files, or layer metadata. Doing so would store sensitive information in plain text on disk, which is undesirable from a security perspective and makes it easier to leak the PIN value.

A better approach is to pass the PIN through the environment only for the build that needs it. This reduces the exposure of the secret and keeps it out of the static build configuration.

To allow BitBake to import the variable from the environment, first extend `BB_ENV_PASSTHROUGH_ADDITIONS`:

```
$ export BB_ENV_PASSTHROUGH_ADDITIONS="$BB_ENV_PASSTHROUGH_ADDITIONS TDX_SIGNED_HSM_TOKEN_PIN"
```

Then run `bitbake`, providing the PIN as an environment variable:

```
$ TDX_SIGNED_HSM_TOKEN_PIN=123456 bitbake tdx-reference-minimal-image
```

With this approach, the PIN is available to the build system only at build time and does not need to be stored in configuration files.

However, keep in mind that although passing the PIN through the environment improves secret handling, it does not by itself provide complete secret protection. Access to the build environment, logs, and CI configuration should still be properly controlled.

For stronger protection, especially in CI or production environments, the PIN should ideally be injected through a dedicated secret-management mechanism, such as CI secret variables, a vault service, or a secure credential store, with appropriate access control and auditing.

## Security considerations

Using an HSM improves key custody by keeping private signing keys outside the build host filesystem, but it does not by itself make the overall secure-boot workflow secure.

A secure deployment still depends on how the signing infrastructure is designed, operated, and controlled. In particular, you should consider, among other things:

- how token PINs and other credentials required to access the HSM are injected into the build system and CI jobs
- how development, test, and production signing environments are separated
- how access to production signing infrastructure is restricted, monitored, and audited
- whether the risk profile of the product requires signing to happen outside the regular build infrastructure

In some cases, even HSM-backed signing integrated into the build system may not be sufficient. For higher-assurance environments, the signing step may need to be performed through a separate offline or air-gapped process, isolated from the main build and CI infrastructure. This can help reduce exposure of signing operations to network-connected systems and provide stronger operational control over release signing.
