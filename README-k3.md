# Bootloader signature checking for TI K3-based platforms

This document describes how bootloader signature checking works for System on Modules (SoMs) that use System on Chips (SoCs) based on the K3 platform by Texas Instruments.

## Introduction

The Texas Instruments K3 platform encompasses a family of SoCs (System on Chips) designed to offer a blend of multicore processing capabilities, optimized for high performance and power efficiency across a broad array of industrial, automotive, and other market segments.

The AM62x family of SoCs (part of the K3 platform) has two types of devices: GP (General Purpose) and HS (High Security).

General Purpose (GP) devices lack several security features and don't support secure boot.

High Security (HS) devices have all security features available and support encrypted and authenticated (secure) boot.

**IMPORTANT**: To have access to the secure boot feature on Verdin AM62, make sure you are using an SoC of type HS.

A High Security (HS) device might be in one of two different states:

- **FS state**: Before enabling secure boot, High Security (HS) devices are in Field Securable (FS) state. In this state, users can run unsigned images, JTAG is available and secure boot is disabled.
- **SE state**: As soon as the keys are programmed in the OTP fuses, the state changes to Security Enforced (SE). In this state, all security policies are applied, secure boot is enforced and JTAG is closed.

For more details on how secure boot works on K3 platforms, it is recommended to read the documentation provided by Texas Instruments. Since most documentation related to security is provided under NDA, you will need to talk to a TI Sales representative.

## Enabling secure boot on Verdin AM62

There are three major steps to enable secure boot on Verdin AM62:

1. Generate the keys and certificates for signing.
2. Sign bootloader artifacts when building the image.
3. Fusing the keys into the SoC.

### Generating keys for signing

In the secure boot context, three different keys play an important role:

- **MEK** (Manufacture Encryption Key): TI signing key used to check the signature of TI signed artifacts.
- **SMPK** (Secondary Manufacture Public Key): user signing key to be used for checking the signature of bootloader artifacts created by the user.
- **BMPK** (Back-up Manufacture Public Key): user backup signing key that can also be used for checking the signature of bootloader artifacts created by the user.

The MEK key comes from TI, it is already burned into the SoC and cannot be changed.

The SMPK key needs to be created by the user and will be used to sign the bootloader artifacts.

BMPK is a backup key.  Its creation and usage are optional (but recommended). In case the main key (SMPK) is lost or compromised, the user is able to switch to this backup key.

For signature checking, the following algorithms are supported:

- RSA (2048 and 4096)
- ECDSA (secp256 and secp521)

Here are some instructions to create the keys. Be aware that this is just an example since you might want to define your own process to create and store the keys.

Export some variables to configure the location and the name of the keys:

```
$ export KEYS_DIR=~/keys/ti
$ export SMPK_NAME=custMpk
$ export BMPK_NAME=backMpk
```

Obs.: In the current implementation, it is mandatory for the SMPK key to be called `custMpk`.

Create a directory to store the keys:

```
$ mkdir -p "${KEYS_DIR}" && cd "${KEYS_DIR}"
```

Create the SMPK key pair and certificate using RSA 4096:

```
$ openssl genrsa -F4 -out ${SMPK_NAME}.key 4096
$ cp ${SMPK_NAME}.key ${SMPK_NAME}.pem
$ openssl req -batch -new -x509 -key ${SMPK_NAME}.key -out ${SMPK_NAME}.crt
```

Create the BMPK key pair and certificate using RSA 4096:

```
$ openssl genrsa -F4 -out ${BMPK_NAME}.key 4096
$ cp ${BMPK_NAME}.key ${BMPK_NAME}.pem
$ openssl req -batch -new -x509 -key ${BMPK_NAME}.key -out ${BMPK_NAME}.crt
```

Additionally, a key called `ti-degenerate-key` needs to be created. This key is only used when building bootloader artifacts for GP (General Purpose) devices, and it is not used in the secure boot implementation, but needs to be there or the U-Boot build will fail:

```
$ openssl genrsa -F4 -out ti-degenerate-key.pem 4096
```

To finish, remove write access to the keys and certificates:

```
$ chmod a-w *
```

### Signing bootloader artefacts

When the `tdx-signed` class is inherited, signing bootloader images on K3-based platforms like AM62 is enabled by default.

A few variables can be used to configure its behavior:

| Variable | Description | Default value |
| :------- | :---------- | :------------ |
| `TDX_K3_HSSE_ENABLE` | Enable/disable secure boot support; allowed values: `0` or `1`. | `1` |
| `TDX_K3_HSSE_KEY_DIR` | Location of the keys and certificates that will be used to sign the bootloader images. See the previous session for an example on how to create the keys. | `${TOPDIR}/keys/ti` |

### Fusing the keys into the SoC

To write to the OTP fuses, a special firmware called OTP Key Writer needs to be created.

OTP Key Writer is a single firmware image that has three components:

- A certificate with the keys and other information to be written to the fuses.
- A secure firmware encrypted and signed by TI (provided under NDA) that will run on the ARM Cortex M4 core, being responsible for writing to the fuses.
- A firmware developed by the user that will run on the ARM Cortex R5 core, being responsible for parsing the certificate and sending messages to the secure firmware, so the keys and other information are written to the OTP fuses.
  
Also, the VPP pin from the SoC requires 1.8v while programming OTP eFuses and should be floating when not programming. That means the hardware needs to be designed in a way that the state of the VPP pin can be controlled by the OTP Key Writer firmware (e.g. via GPIO pin).

The [OTP Keywriter Tutorial](https://dev.ti.com/tirex/explore/node?node=A__AagJ-8QGXM582KzTgxFZbA__AM62-ACADEMY__uiYMDcq__LATEST) provided by TI might help to understand how to implement the OTP KeyWriter software.

Additional documentation and sample code are provided by Texas Instruments, but only under NDA. It is recommended to get in touch with a TI Sales representative to get access to it.
