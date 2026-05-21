# Secure Debug

This document describes how the `tdx-secure-debug` class restricts JTAG/debug access on Toradex SoMs at production time, and how to configure it.

## Introduction

Modern SoCs ship with a JTAG interface that is fully enabled out of reset. Left as-is, this interface lets an attacker with physical access halt the CPU, read memory (including secrets that would otherwise be protected by secure boot), and inject code. For products that have already enabled secure boot (HAB/AHAB), an open JTAG is the most direct way around it.

Many NXP iMX SoCs — the iMX6, iMX7, iMX8M, iMX8 and iMX8X families — implement a hardware block called the **System JTAG Controller (SJC)** that gates the JTAG interface. iMX9x SoCs use a different mechanism (the EdgeLock Secure Enclave, ELE) and are handled by a separate backend. The SJC supports the following operating modes, selected by One-Time Programmable (OTP) e-fuses:

- **JTAG Enable** – default state; JTAG fully open. Used during development.
- **Secure JTAG (challenge/response)** – JTAG access is gated by a response value programmed into fuses. A debug probe must answer a hardware challenge with the matching response before access is granted. The exact model varies by family: on iMX6/7/8M the response is a single 56-bit secret; on iMX8/iMX8X the device exposes a 64-bit challenge and accepts two separate keys (an OEM/normal-world key and a TrustZone/secure-world key, each 128 bits), with provisioning tied to the OEM Open/Closed lifecycle.
- **No Debug** – security-sensitive debug features (CPU halt, memory access, etc.) are disabled. Some lower-risk features (e.g. boundary scan) may remain available.
- **JTAG Disabled** – the JTAG interface is fully disabled. Even boundary scan is unavailable.

The rest of this section describes the iMX6/7/8M variant, which is what the current implementation supports. In addition to the mode, three related fuses are relevant:

- **JTAG_HEO** – when set, blocks the path by which HAB can re-enable JTAG from trusted boot software (`HAB_JDE`). This prevents a signed but malicious boot stage from reopening JTAG. Relevant only in **Secure JTAG** mode, and only on iMX6/7/8M — the iMX8/iMX8X model does not use the same HAB software reopen path.
- **KTE** (Kill Trace Enable) – when set, gates the on-chip bus-tracing path (ETM and related modules) by the SJC security state. Without KTE, an attacker with physical access can attach a trace probe and observe CPU execution flow even while JTAG itself is restricted — a side channel that's adjacent to JTAG but not covered by `JTAG_SMODE` alone. The SRM pairs `JTAG_SMODE+KTE` as one disable group; leaving KTE clear results in an incomplete configuration.
- **BOOT_CFG_LOCK** – locks the boot-configuration fuse region (which includes `JTAG_SMODE`, `SJC_DISABLE`, `JTAG_HEO`, and `KTE`) against shadow-register overrides. The class always sets this lock in **OP** (Override-Protect) mode, which still allows the SEC_CONFIG[1] close fuse to be burned afterwards.

The `tdx-secure-debug` class abstracts these fuses behind a small set of `TDX_SECURE_DEBUG_*` variables, generates the corresponding fuse-programming commands at build time, and appends them to the existing `fuse-cmds.txt` / `imx-config.fuse` files produced by the HAB flow.

## Supported SoCs

| SoC family   | Backend | Status         |
| :----------- | :------ | :------------- |
| iMX8M Mini   | SJC     | Supported      |
| iMX8M Nano / Quad | SJC | Planned        |
| iMX8M Plus   | SJC     | Not supported (SoC erratum: Secure Debug is not functional on this part) |
| iMX6 / iMX7  | SJC     | Planned        |
| iMX8 / iMX8X | SJC     | Planned (more elaborate variant: 64-bit challenge, two response keys, lifecycle-tied provisioning) |
| iMX9x (iMX93, iMX95) | ELE | Planned (credential/certificate-based, not fuse-based) |
| TI K3 (AM6x, e.g. AM62, AM69) | TI K3 | Planned (X.509 debug certificate validated by System Firmware) |

The class is currently restricted by an explicit machine allow-list (`verdin-imx8mm`). Building for any other machine with `TDX_SECURE_DEBUG_ENABLE=1` will fail at sanity-check time.

## Prerequisites

- Secure boot must be enabled (`TDX_IMX_HAB_ENABLE=1`). The class refuses to build if HAB is disabled, since programming a JTAG response key without secure boot offers little protection (any unsigned image can simply read or overwrite the fuses' shadow registers before they are locked).
- For **Secure JTAG** mode, a 56-bit key file must be provided by the user (see *Key file* below). The class does **not** auto-generate this key; the build fails if the file does not exist.

## Configuring secure debug

Inherit the class and set the configuration variables in your distro config or `local.conf`:

```bash
INHERIT += "tdx-signed"
INHERIT += "tdx-secure-debug"

TDX_SECURE_DEBUG_MODE = "authenticated"
TDX_SECURE_DEBUG_KEY_FILE = "/path/to/keys/debug/key.txt"
```

### Variables

| Variable | Description | Default value |
| :------- | :---------- | :------------ |
| `TDX_SECURE_DEBUG_ENABLE` | Enable/disable secure-debug fuse generation. Allowed values: `0` or `1`. | `1` |
| `TDX_SECURE_DEBUG_MODE` | Operating mode. Allowed values: `authenticated` (Secure JTAG, challenge/response) or `disabled` (No Debug). | `authenticated` |
| `TDX_SECURE_DEBUG_KEY_FILE` | Path to the file containing the 56-bit response key, used when `TDX_SECURE_DEBUG_MODE="authenticated"`. The file must exist; the build fails otherwise. | `${TOPDIR}/keys/debug/key.txt` |
| `TDX_SECURE_DEBUG_SJC_HEO` | Block the HAB software path (`HAB_JDE`) that can re-enable JTAG from trusted boot software. Recommended to keep enabled. Allowed values: `0` or `1`. | `1` |
| `TDX_SECURE_DEBUG_SJC_DISABLE` | Fully disable the JTAG interface. When set to `1`, this overrides `TDX_SECURE_DEBUG_MODE` and selects the strongest available full-disable setting for the SoC. Allowed values: `0` or `1`. | `0` |

### Operating modes

The combination of `TDX_SECURE_DEBUG_MODE` and `TDX_SECURE_DEBUG_SJC_DISABLE` selects one of three end states:

| `MODE` | `SJC_DISABLE` | End state | Fuses programmed |
| :----- | :------------ | :-------- | :--------------- |
| `authenticated` | `0` | Secure JTAG (challenge/response) | `SJC_RESP[55:0]`, `SJC_RESP_LOCK`, `JTAG_SMODE=01`, optionally `JTAG_HEO`, `KTE`, then `BOOT_CFG_LOCK=OP` |
| `disabled`      | `0` | No Debug                         | `JTAG_SMODE=11`, `KTE`, then `BOOT_CFG_LOCK=OP` |
| any             | `1` | JTAG fully disabled              | `SJC_DISABLE=1`, then `BOOT_CFG_LOCK=OP` |

**Recommendation**: for production devices that need post-deployment debugging (e.g. failure analysis returns), use `authenticated`. For devices that never need debugging again, use `SJC_DISABLE=1`.

`KTE` is set implicitly whenever `SJC_DISABLE=0` (i.e. in `authenticated` and `disabled` modes); it is not exposed as a user variable because NXP's secure-configuration guidance is unambiguous and leaving it clear creates an unrestricted trace-port side channel. When `SJC_DISABLE=1`, the trace path is already cut as part of the full JTAG disable, so `KTE` is redundant and is not emitted.

### Key file

When `TDX_SECURE_DEBUG_MODE="authenticated"`, the file pointed to by `TDX_SECURE_DEBUG_KEY_FILE` must contain a 56-bit secret key in hexadecimal, with the following format:

- Exactly **14 hex characters** (`0`–`9`, `a`–`f`, `A`–`F`).
- **No `0x` prefix.**
- **No spaces, separators, or other whitespace** (a single trailing newline is allowed).

Example contents of `key.txt`:

```
112233445566aa
```

The key is the value that a debug probe must supply to answer the SJC challenge. It is **not** auto-generated by the build; the user is responsible for choosing a strong random key (typically from a secure RNG) and storing it safely. The recommended workflow is to keep the key in the same secure key-management system used for the HAB signing keys.

The key is split across two fuse words by the build:

- Word `8:0` ← bits `[31:0]`  (lower 32 bits → last 8 hex chars of the key)
- Word `8:1` ← bits `[55:32]` (upper 24 bits → first 6 hex chars of the key, zero-padded to 32 bits)

If the file is missing, contains the wrong number of characters, or contains non-hex characters, the build fails with an explicit error before any artifact is produced.

### Closing the device

If `TDX_SECURE_DEBUG_ENABLE=1`, at the end of the build the secure-debug fuse commands are appended to the existing `fuse-cmds.txt` and `imx-config.fuse` files in the image deploy directory, between the SRK-hash writes and the `SEC_CONFIG[1]` close command. The ordering matters: response-key fuses must be written before the device is closed.

Example of the appended section (Secure JTAG mode, with `JTAG_HEO=1`):

```
$ cat deploy/images/verdin-imx8mm/fuse-cmds.txt
[...SRK hash writes...]

# === Secure Debug fuses ===
# These fuses configure the System JTAG Controller (SJC).
# They are One-Time Programmable e-fuses; the BOOT_CFG_LOCK at
# the end protects the JTAG mode bits against shadow-register override.
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

# After the device successfully boots a signed image without generating
# any HAB events, it is safe to secure, or 'close', the device.
[...]
fuse prog -y 1 3 0x02000000
```

The commands are executed in the U-Boot command-line interface, in order. Read the warning messages carefully: these are One-Time Programmable e-fuses, and the operations are **irreversible**.

## Validating secure debug

After programming the fuses and rebooting the device, the resulting state can be verified:

- For Secure JTAG, attempt to attach a debug probe **without** providing the response key – it must fail. Then attempt with the correct key – it must succeed.
- For No Debug or full disable, attaching a probe must fail unconditionally.

If `JTAG_HEO=1`, even a signed-and-running boot stage cannot reopen JTAG via the HAB software path.

## Known issues / caveats

- Currently only `verdin-imx8mm` is in the supported machine list; other iMX8M variants will be added as their fuse maps are validated against the corresponding NXP reference manuals.
- The class does **not** auto-generate the response key. This is intentional: silently generating a debug key would invite weak or lost keys.
- The `BOOT_CFG_LOCK=OP` write locks **all** boot-configuration fuses in the 0x470–0x4B0 range against shadow-register overrides, not just the JTAG mode bits. This is the intended behavior at the end of the close process.
- **`BOOT_CFG_LOCK` is set to OP only (value `10`, mask `0x00000008`), not OP+WP (value `11`, mask `0x0000000C`).** This is deliberate. Per the SRM, OP prevents shadow-register override but still allows the field to be burned; WP prevents burning. The secure-debug fuses are written **before** the `SEC_CONFIG[1]` close fuse, and `SEC_CONFIG[1]` is itself located at `0x470[25]` — inside the range protected by `BOOT_CFG_LOCK`. Setting `BOOT_CFG_LOCK=OP+WP` here would therefore block the subsequent close write. OP alone is also sufficient for the threat model: it prevents an attacker from temporarily overriding `JTAG_SMODE`/`SJC_DISABLE`/`JTAG_HEO` via the shadow register to re-open JTAG, which is the relevant access-control concern. WP would only add DoS-resistance against signed code burning further bits in this range.
- `TDX_SECURE_DEBUG_SJC_DISABLE=1` is the strongest setting available but it also disables boundary scan and other lower-risk JTAG features. Use only when no post-deployment access is required.

## References

- *Security Reference Manual for i.MX 8M Mini Applications Processor*, NXP, Rev. 1, 01/2024 (chapter 2.11 "System JTAG Controller (SJC)" and chapter 7 "Fusemap").
- NXP Application Note AN4581, *Secure JTAG for i.MX RT and i.MX Application Processors* (general SJC operating principles).
