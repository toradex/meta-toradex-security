# Secure Debug

Modern SoCs usually expose a JTAG/debug interface that is enabled by default. If left open in production, this interface may allow an attacker with physical access to halt the CPU, inspect memory, extract secrets, or inject code.

This is especially important for devices that use features such as secure boot and data-at-rest encryption. An open debug interface can become a practical way to bypass those protections.

To mitigate this risk, this layer provides a feature called **Secure Debug**.

Secure Debug is currently supported on the following SoMs:

- Apalis iMX6
- Apalis iMX8
- Colibri iMX6DL
- Colibri iMX6ULL (1GB eMMC variant only)
- Colibri iMX7D (1GB eMMC variant only)
- Colibri iMX8X
- SMARC iMX8MP
- Verdin iMX8MM
- Verdin iMX8MP

Support for additional SoMs and SoC families is planned. Since each SoC family may use a different hardware mechanism to restrict debug access, new platforms may introduce additional variables or different provisioning requirements.

## Overview

Supporting secure debug requires three things from the SoC: a hardware block able to gate the debug port, a persistent place to record the desired policy (e.g. OTP e-fuses), and an authentication mechanism that can reopen debug access to whoever holds the right secret. Vendors implement all three differently. For example, NXP uses a symmetric challenge/response on its older families and a signed, credential-based exchange on the newer ones.

The layer therefore exposes a small, policy-oriented interface that stays the same across SoC families, and delegates the hardware specifics to a per-family backend selected automatically from the machine. The user chooses *what* the device should allow, and the backend decides *how* that is achieved on the target.

When secure debug is enabled, provisioning data is generated at build time. For example, on iMX8-based SoCs the layer generates the required fuse commands and appends them to the `fuse-cmds.txt` and `imx-config.fuse` files already produced by the HAB/AHAB flow. Nothing is programmed by the build itself. The commands are executed later, by the user, on the device.

The SoMs currently supported are based on NXP iMX6, iMX7, iMX8M, iMX8 and iMX8X SoCs, which use the System JTAG Controller (SJC) backend. For details on the SJC backend, including its configuration variables, key management and provisioning, see the [README-secure-debug-sjc.md](README-secure-debug-sjc.md) file.

## Enabling Secure Debug

To enable Secure Debug, inherit the `tdx-secure-debug` class in your distro configuration or `local.conf`:

```bash
INHERIT += "tdx-secure-debug"
```

Secure Debug requires secure boot to be enabled. The build fails if Secure Debug is enabled without secure boot.

If Secure Debug is enabled on unsupported machines, the build fails at sanity-check time with an explicit error, so an unsupported target cannot silently produce an image without the expected provisioning data.

## Configuration variables

The following generic variables are available:

| Variable | Description | Default value |
| :------- | :---------- | :------------ |
| `TDX_SECURE_DEBUG_ENABLE` | Enable or disable the Secure Debug feature. Allowed values: `0` or `1`. | `1` |
| `TDX_SECURE_DEBUG_MODE` | Debug policy. Allowed values: `authenticated` or `no-debug`. | `authenticated` |

Each backend adds its own variables. For SJC-based SoCs, see [Configuration variables](README-secure-debug-sjc.md#configuration-variables) in the SJC documentation.

To disable security-sensitive debug access instead of using authenticated JTAG:

```bash
INHERIT += "tdx-secure-debug"

TDX_SECURE_DEBUG_MODE = "no-debug"
```

## Board-level prerequisites

Authenticated debug also depends on the carrier board exposing the debug interface, which is outside the control of this layer. Check this before concluding that a fused device is faulty.

## Verifying authenticated debug

Debug probe support for the authentication flow differs between SoCs even within the same vendor tooling, and is the most common obstacle to verifying this feature. Confirm that your probe implements the mechanism for your SoC **before** programming any fuse, ideally on a device you can afford to lose debug access to.

## Limitations

- Only the SJC backend is implemented, covering iMX6, iMX7, iMX8M, iMX8 and iMX8X. The iMX9x EdgeLock Secure Enclave and TI K3 use different mechanisms and are not supported yet.
