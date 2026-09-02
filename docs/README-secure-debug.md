# Secure Debug

Modern SoCs usually expose a JTAG/debug interface that is enabled by default. If left open in production, this interface may allow an attacker with physical access to halt the CPU, inspect memory, extract secrets, or inject code.

This is especially important for devices that use features such as secure boot and data-at-rest encryption. An open debug interface can become a practical way to bypass those protections.

To mitigate this risk, this layer provides a feature called **Secure Debug**.

Secure Debug is currently supported on the following SoMs:

- SMARC iMX8MP
- Verdin iMX8MM
- Verdin iMX8MP

On the iMX8MP-based SoMs only the non-authenticated modes are available, because NXP defeatured the Secure JTAG mode on that SoC. See [Limitations](#limitations).

Support for additional SoMs and SoC families is planned. Since each SoC family may use a different hardware mechanism to restrict debug access, new platforms may introduce additional variables or different provisioning requirements.

## Overview

Supporting secure debug requires three things from the SoC: a hardware block able to gate the debug port, a persistent place to record the desired policy (e.g. OTP e-fuses), and an authentication mechanism that can reopen debug access to whoever holds the right secret. Vendors implement all three differently. For example, NXP uses a symmetric challenge/response on its older families and a signed, credential-based exchange on the newer ones.

The layer therefore exposes a small, policy-oriented interface that stays the same across SoC families, and delegates the hardware specifics to a per-family backend selected automatically from the machine. The user chooses *what* the device should allow, and the backend decides *how* that is achieved on the target.

When secure debug is enabled, provisioning data is generated at build time. For example, on iMX8-based SoCs the layer generates the required fuse commands and appends them to the `fuse-cmds.txt` and `imx-config.fuse` files already produced by the HAB/AHAB flow. Nothing is programmed by the build itself. The commands are executed later, by the user, on the device.

## Secure Debug on iMX8M

On iMX8M, Secure Debug uses the NXP **System JTAG Controller (SJC)**. The SJC can gate the JTAG interface and require a challenge/response authentication flow before allowing debug access.

The supported operating modes are:

- **JTAG Enabled**: JTAG is fully open. This is the default state and is normally used during development.
- **Secure JTAG**: JTAG access is blocked after reset and can only be reopened by a debugger that knows the programmed response key.
- **No Debug**: security-sensitive debug features, such as CPU halt and memory access, are disabled. Some lower-risk JTAG features, such as boundary scan, may still remain available depending on the SoC.

Not every SoC of the family supports every mode. On the iMX8MP, "Secure JTAG" is not available and only the "No Debug" operating mode can be used. See [Limitations](#limitations).

In addition to these operating modes, the SJC backend also provides an option to fully disable the JTAG controller. When enabled, all JTAG functionality is disabled, including boundary scan.

Besides the selected mode, the backend also configures whether the HAB software path can reopen JTAG from trusted boot code, whether the trace port is gated by the debug policy, and the lock bits that prevent the configuration from being overridden later.

Authentication is per session. After every reset, the SJC returns to the locked state, so the debugger must repeat the challenge/response authentication flow.

## Enabling Secure Debug

To enable Secure Debug, inherit the `tdx-secure-debug` class in your distro configuration or `local.conf`:

```bash
INHERIT += "tdx-secure-debug"
```

Secure Debug requires secure boot to be enabled. The build fails if Secure Debug is enabled without secure boot, because programming a debug response key without secure boot offers limited protection: an unsigned image could still access or modify fuse shadow registers before they are locked.

If Secure Debug is enabled on unsupported machines, the build fails at sanity-check time with an explicit error, so an unsupported target cannot silently produce an image without the expected provisioning data.

## Configuration variables

The following generic variables are available:

| Variable | Description | Default value |
| :------- | :---------- | :------------ |
| `TDX_SECURE_DEBUG_ENABLE` | Enable or disable the Secure Debug feature. Allowed values: `0` or `1`. | `1` |
| `TDX_SECURE_DEBUG_MODE` | Debug policy. Allowed values: `authenticated` or `no-debug`. | `authenticated` |

For SJC-based SoCs (e.g. iMX8M), the following additional variables are available:

| Variable | Description | Default value |
| :------- | :---------- | :------------ |
| `TDX_SECURE_DEBUG_KEY_FILE` | Path to the file containing the SJC response key, used when `TDX_SECURE_DEBUG_MODE = "authenticated"`. | `${TOPDIR}/keys/secure-debug/key.txt` |
| `TDX_SECURE_DEBUG_SJC_HEO` | Block the HAB software path that can reopen JTAG from trusted boot software without the challenge/response. Only applies in `authenticated` mode. Allowed values: `0` or `1`. | `1` |
| `TDX_SECURE_DEBUG_SJC_DISABLE` | Fully disable the JTAG interface. When set to `1`, this overrides `TDX_SECURE_DEBUG_MODE` and selects the strongest available full-disable setting for the SoC. Allowed values: `0` or `1`. | `0` |

`TDX_SECURE_DEBUG_SJC_HEO` is programmed only in `authenticated` mode. NXP scopes the HAB unlock command to `JTAG_SMODE = Secure` in the SoC security reference manual, so the fuse is not emitted for the other policies.

Example configuration:

```bash
INHERIT += "tdx-secure-debug"

TDX_SECURE_DEBUG_MODE = "authenticated"
TDX_SECURE_DEBUG_KEY_FILE = "${TOPDIR}/keys/secure-debug/key.txt"
```

To disable security-sensitive debug access instead of using authenticated JTAG:

```bash
INHERIT += "tdx-secure-debug"

TDX_SECURE_DEBUG_MODE = "no-debug"
```

To fully disable JTAG on SJC-based SoCs:

```bash
INHERIT += "tdx-secure-debug"

TDX_SECURE_DEBUG_SJC_DISABLE = "1"
```

## Key management

When `TDX_SECURE_DEBUG_MODE = "authenticated"`, `TDX_SECURE_DEBUG_KEY_FILE` must point to a file containing the SJC response key in hexadecimal format.

For iMX8M, the key is a 56-bit value represented by exactly 14 hexadecimal characters, without a `0x` prefix. Example:

```text
7a73cfcdb180e3
```

A trailing newline is allowed. Spaces, separators, and non-hexadecimal characters are not allowed. The build fails if the file is missing, has the wrong length, or contains invalid characters.

The key is **not** generated automatically. Generate it with a secure random-number generator and protect it like a production signing key:

- do not commit it to the repository;
- do not expose it in build logs;
- store it with appropriate access control;
- back it up securely before provisioning devices.

The response key is programmed into OTP fuses. Once programmed and locked, it cannot be changed or recovered by software. If the key is lost, authenticated debug access to that device is lost as well.

This build-time flow uses the same response key for all devices programmed from the same build. If that key leaks, every device provisioned with it can be opened through authenticated JTAG.

If your threat model requires per-device debug credentials, provision the response key in your manufacturing flow instead of using this class to generate the response-key fuse commands.

The response key also appears in clear text in the artifacts the build produces: `fuse-cmds.txt` and `imx-config.fuse` in the deploy directory contain the key as fuse values. This is unavoidable, since the fuse commands *are* the key. Treat the deploy directory as sensitive: archived images and CI artifact bundles are the most likely way for the key to escape.

## Provisioning

When Secure Debug is enabled, the build appends a dedicated section to `fuse-cmds.txt`, between the secure boot SRK hash commands and the command that closes the device.

Example:

```text
$ cat deploy/images/verdin-imx8mm/fuse-cmds.txt
[...]

# === Secure Debug fuses ===
# These fuses configure the System JTAG Controller (SJC).
# Program them exactly in the order shown below.
# SJC_RESP[31:0]
fuse prog -y 8 0 0x445566aa
# SJC_RESP[55:32]
fuse prog -y 8 1 0x00112233
# SJC_RESP_LOCK
fuse prog -y 0 0 0x00000400
# JTAG_SMODE = Secure JTAG
fuse prog -y 1 3 0x00400000
# JTAG_HEO = 1 (block HAB software reopen)
fuse prog -y 1 3 0x04000000
# KTE = 1 (gate bus tracing on SJC state)
fuse prog -y 1 3 0x00100000
# BOOT_CFG_LOCK = OP (override-protect JTAG mode)
fuse prog -y 0 0 0x00000008

[...]
```

The same fuses are also recorded in `imx-config.fuse`, which provides a map of the fuses that will be programmed. Unlike `fuse-cmds.txt`, `imx-config.fuse` is not an ordered programming script.

Program the commands manually in U-Boot, exactly in the order shown in `fuse-cmds.txt`.

> **Warning**
>
> Fuse programming is irreversible. Review the generated commands carefully before executing them.

Before programming the Secure Debug fuses, make sure that:

- secure boot is enabled;
- the signed image boots correctly;
- the generated `fuse-cmds.txt` was reviewed;
- the Secure Debug key is backed up securely;
- the debug probe and authentication flow were tested on a non-production device;
- all Secure Debug fuses are programmed before closing the device.

The ordering of the generated commands is important:

- The response key is programmed and locked before Secure JTAG mode is enabled.
- The debug configuration is locked last, after every mode fuse has been programmed.
- The Secure Debug fuses must be programmed before closing the device.

On a closed device, the hardened U-Boot command policy blocks fuse programming. Therefore, a device that is closed before the Secure Debug fuses are programmed cannot be provisioned for Secure Debug later.

## Verifying authenticated debug

After programming the fuses and power-cycling the board, verify the result with a debug probe that supports the SoC authentication mechanism, such as Lauterbach TRACE32.

For authenticated mode, the expected behavior is:

1. Attach without supplying the response key. This must fail.
2. Attach with the correct response key. This must succeed.
3. Attach with an intentionally wrong response key. This must fail.

Flipping a single bit of the correct key is a useful negative test, because it helps confirm that the full response key is being checked.

## Limitations

- Authenticated Secure Debug is not available on the iMX8MP, even though the SoC belongs to the iMX8M family. Erratum ERR052318 states that in Secure JTAG mode the SJC "does not correctly control JTAG access and may not unlock the device for JTAG access", and AN4686 states that "the Secure Debug mode is not functional on the i.MX 8M Plus". NXP defeatured the mode and recommends "No Debug" mode or disabling the SJC instead, which is what the layer offers on this SoC.
- Only the SJC backend is implemented, and within it only iMX8M is supported. The two-key iMX8/iMX8X variant, the iMX9x EdgeLock Secure Enclave, and TI K3 use different mechanisms and are not supported yet.
- All devices programmed from the same build share the same response key. See [Key management](#key-management).

## References

The implementation was based on the following documents. Access to some of them may be restricted and require a non-disclosure agreement with the SoC vendor.

- AN4686 — *Secure Debug in i.MX 6/7/8M Family of Applications Processors*, Rev. 4.0, 5 February 2025
- *IMX8MP_1P33A Mask Set Errata*, Rev. 2.1, 2 October 2024
- *Security Reference Manual for i.MX8M Mini Applications Processor*, Rev. 1, January 2024
- *Security Reference Manual for i.MX 8M Plus Applications Processor*, Rev. 0, April 2021
