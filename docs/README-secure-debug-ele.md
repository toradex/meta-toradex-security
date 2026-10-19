# Secure Debug on NXP iMX EdgeLock Secure Enclave (ELE)

This document describes the Secure Debug backend for SoMs based on the NXP iMX9x SoCs, where debug access is controlled by the EdgeLock Secure Enclave (ELE). For an overview of the Secure Debug feature and how to enable it, see the [README-secure-debug.md](README-secure-debug.md) file.

## Secure Debug with the ELE

On ELE-based SoCs, debug access is controlled by the **EdgeLock Secure Enclave (ELE)**. There is no response key in fuses. Instead, the policy follows the AHAB lifecycle, and debug is reopened with a signed credential rooted in the secure boot keys.

- While the device is **OEM Open**, debug is available without authentication.
- Once the device is **OEM Closed** (`ahab_close`), debug is blocked until a debugger authenticates with a **debug credential** (DC).

A debug credential is a certificate signed with one of the secure boot keys (SRK). It carries the public part of a **debug credential key** (DCK) owned by the developer, the list of debug domains it opens, and optionally the UUID of the one device it is valid for.

To open a debug session, the debugger requests a challenge from the ELE through the debug mailbox, and answers it with the credential and a signature made with the DCK private key. The ELE checks both signatures before opening the requested debug domains. The whole exchange is performed by the NXP SPSDK tools (`nxpdebugmbox`) together with a supported debug probe.

In consequence, **authenticated debug requires no fuses**. Closing the device is enough, and the credential can be created at any time later, as long as the secure boot keys are available. The build generates configuration templates for the SPSDK tools to help with this. See [Debug credentials in authenticated mode](#debug-credentials-in-authenticated-mode).

Debug access is split into debug domains, each with its own set of debug permissions (secure and non-secure, invasive and non-invasive debug). The debug domains opened by a debug credential are selected by its `cc_socu` field, which holds one 4-bit nibble per debug domain. The available debug domains depend on the SoC.

The `no-debug` policy programs the debug-disable fuses of every OEM debug domain. Debug then remains blocked even for a valid debug credential, while boundary scan remains available. A stronger option also disables the JTAG controller itself, boundary scan included.

### Debug domains on iMX95

On the iMX95 the OEM debug domains are the Cortex-M33, the trace memory controller (ETR), the Cortex-M7, the DAP AHB-AP, and the application domain, which covers the Cortex-A55 cores, the GPU and the camera subsystem. They are selected by the following `cc_socu` bits:

| Bits | Debug domain |
| :--- | :----------- |
| 7:4 | Cortex-M33 |
| 11:8 | Trace memory controller (ETR) |
| 15:12 | Cortex-M7 |
| 19:16 | DAP AHB-AP |
| 23:20 | Application domain (Cortex-A55 cores, GPU and camera subsystem) |

The other bits select debug domains owned by NXP, which cannot be opened with the OEM keys. Within each nibble, bit 3 enables secure invasive debug (SPIDEN), bit 2 secure non-invasive debug such as trace (SPNIDEN), bit 1 non-secure invasive debug (DBGEN) and bit 0 non-secure non-invasive debug (NIDEN). For example, `0x00FFFFF0` opens all OEM debug domains, `0x00F00000` opens only the application domain, and `0x00300000` opens the application domain for normal-world debug only.

## Configuration variables

For supported ELE-based SoCs, the following additional variables are available:

| Variable | Description | Default value |
| :------- | :---------- | :------------ |
| `TDX_SECURE_DEBUG_ELE_DC_CC_SOCU` | Debug domains and debug permissions granted by the generated debug credential (`cc_socu` field), as a hexadecimal value with one nibble per debug domain. Only OEM debug domains can be selected. | all OEM domains (`0x00FFFFF0` on iMX95) |
| `TDX_SECURE_DEBUG_ELE_DC_UUID` | UUID of the device the generated debug credential is bound to, as 32 hexadecimal characters. When empty, the template carries a placeholder to be replaced for each device. | empty |
| `TDX_SECURE_DEBUG_ELE_DCK_KEY` | Path to the private key of the debug credential key (DCK) pair, referenced by the generated templates. | `${TOPDIR}/keys/secure-debug/dck.pem` |
| `TDX_SECURE_DEBUG_ELE_JTAG_DISABLE` | Fully disable the JTAG interface, including boundary scan and the authenticated debug flow. When set to `1`, this overrides `TDX_SECURE_DEBUG_MODE`. Allowed values: `0` or `1`. | `0` |

Example enabling authenticated debug to generate credential templates for a device with a particular UUID:

```bash
INHERIT += "tdx-secure-debug"

TDX_SECURE_DEBUG_MODE = "authenticated"
TDX_SECURE_DEBUG_ELE_DC_UUID = "00112233445566778899aabbccddeeff"
```

Example to fully disable JTAG:

```bash
INHERIT += "tdx-secure-debug"

TDX_SECURE_DEBUG_ELE_JTAG_DISABLE = "1"
```

## Debug credentials in authenticated mode

In authenticated mode, no debug secret is programmed into the device, and the build does not create any credential. It generates two configuration templates for the NXP SPSDK tools (version 3.x) in the deploy directory, under `secure-debug/`:

- `dc.yaml`, used to create the debug credential. It references the SRK certificate and private key of the secure boot PKI (`TDX_IMX_HAB_CST_DIR`, `TDX_IMX_HAB_CST_SRK_INDEX`), the DCK set in `TDX_SECURE_DEBUG_ELE_DCK_KEY`, the debug domains and the device UUID.
- `dat.yaml`, used to authenticate a debug session with that credential. It references the DCK and the SRK table of the secure boot PKI.

The templates contain paths, not keys. When the secure boot keys are stored in an HSM (`TDX_SIGNED_HSM = "1"`), `dc.yaml` accesses the SRK private key through the SPSDK PKCS#11 plugin instead, using the token and object of `TDX_IMX_HAB_CST_SRK_CERT` and the module set in `TDX_SIGNED_HSM_PKCS11_MODULE_PATH`. See [Creating the credential with an HSM](#creating-the-credential-with-an-hsm).

Authenticated debug on the iMX95 requires a secure boot PKI with **ECC keys and without the CA flag** on the SRKs (for example, created with `ahab_pki_tree.sh -kt ecc -kl p384 -da sha384 -srk-ca n`), and a DCK of the **same key type** as the SRKs. See [Limitations](#limitations).

The DCK is not generated automatically. Create it once, with the same key type as the SRKs, and protect it like a signing key. For example, for p384 SRKs:

```bash
openssl ecparam -name secp384r1 -genkey -noout -out keys/secure-debug/dck.pem
```

The DCK can also be stored in an HSM, by replacing the `signer` entry of `dat.yaml` with a PKCS#11 signer as described in [Creating the credential with an HSM](#creating-the-credential-with-an-hsm).

The UUID of a device can be read with `nxpdebugmbox -i <probe interface> tool get-uuid -f <family>`, where the family is `mimx9596` for the iMX95.

Using the templates generated by the build, the authenticated debug credential can be created with the following command:

```bash
cd deploy/images/<machine>/secure-debug
nxpdebugmbox dat dc export -c dc.yaml -o dc.bin
```

Creating the credential requires the SRK private key. When the key file is encrypted, as in a PKI tree created by the NXP CST scripts, the SPSDK tools prompt for its password.

Whoever holds a debug credential and its DCK private key can open debug on the devices the credential is valid for. A credential with an all-zero UUID opens every device fused with the same secure boot keys, so prefer one credential per device.

It is recommended to use SPSDK 3.x. Earlier versions use a different configuration format, and create iMX95 debug credentials whose permission field does not match the one documented for the ELE.

### Creating the credential with an HSM

With HSM signing, install the SPSDK PKCS#11 plugin (`pip install spsdk-pkcs11`), provide the token PIN in the `SPSDK_HSM_PIN` environment variable, and make sure the PKCS#11 module path in `dc.yaml` is valid on the host running the SPSDK tools.

```bash
export SPSDK_HSM_PIN=<token PIN>
nxpdebugmbox dat dc export -c dc.yaml -o dc.bin
```

The PIN is read from the environment so that it does not appear in the generated templates.

The SRK private key can be selected in `TDX_IMX_HAB_CST_SRK_CERT` either by label (`object=`) or by ID (`id=`).

Even with an HSM, `dat.yaml` references the four SRK certificates as files in `TDX_IMX_HAB_CST_CERTS_DIR`, named after `TDX_IMX_HAB_CST_CRYPTO`, `TDX_IMX_HAB_CST_KEY_SIZE`, `TDX_IMX_HAB_CST_DIG_ALGO` and `TDX_IMX_HAB_CST_SRK_CA`, and takes the CA flag of the SRK table from `TDX_IMX_HAB_CST_SRK_CA`. Copy the SRK certificates to that directory and set these variables to describe the PKI stored in the HSM, or edit `dat.yaml` accordingly.

## Provisioning in `no-debug` mode

In `no-debug` mode (or when JTAG is disabled via `TDX_SECURE_DEBUG_ELE_JTAG_DISABLE`), the build appends a dedicated section to `fuse-cmds.txt`, between the secure boot SRK hash commands and the command that closes the device.

Example for Verdin iMX95:

```text
$ cat deploy/images/verdin-imx95/fuse-cmds.txt
[...]

# === Secure Debug fuses ===
# These fuses configure the debug policy enforced by the EdgeLock
# Secure Enclave (ELE). Program them before running 'ahab_close'.
# Disable debug domains: CM33 ETR CM7 DAP_AHB_AP
fuse prog -y 7 0 0x0000FFFF
# Disable debug domains: APPS
fuse prog -y 7 1 0x0000000F

[...]
```

With `TDX_SECURE_DEBUG_ELE_JTAG_DISABLE = "1"`, a command to set the JTAG disable fuse is also added to `fuse-cmds.txt`.

Program the commands manually in U-Boot, exactly in the order shown in `fuse-cmds.txt`.

> **Warning**
>
> Fuse programming is irreversible. Review the generated commands carefully before executing them.

Before programming the Secure Debug fuses, make sure that:

- secure boot is enabled;
- the signed image boots correctly;
- the generated `fuse-cmds.txt` was reviewed;
- the debug credential key (DCK) is backed up securely;
- the debug probe and authentication flow were tested on a non-production device;
- all Secure Debug fuses are programmed before closing the device.

On a closed device, the hardened U-Boot command policy blocks fuse programming. Therefore, a device that is closed before the Secure Debug fuses are programmed cannot be provisioned for Secure Debug later.

## Verifying authenticated debug

Verify authenticated debug only after the device has been closed with `ahab_close`. The debug mailbox used for authentication is only available once the device is OEM Closed.

Then authenticate with the generated `dat.yaml`:

```bash
nxpdebugmbox -i <probe interface> dat auth -c dat.yaml
```

After authenticating, you can attach the debugger.

On the iMX95 with ELE firmware 2.0.5, authentication only works in serial download mode (see [Limitations](#limitations)). Boot the device in serial download mode, authenticate, and then load the signed bootloader over USB without resetting the device, for example with `uuu -b spl imx-boot`. Debug stays open until the next reset.

Make sure to do the negative tests as well, using a credential bound to another UUID and a credential signed with an SRK that does not match the fuses.

In `no-debug` mode, authentication with a valid credential must not open the disabled domains.

## Limitations

The following limitations are currently known:

- On the iMX95, RSA keys are not supported for debug authentication. With RSA keys, the response to the ELE challenge exceeds the maximum size the ELE accepts, and the ELE aborts the session and resets the SoC. ECC SRKs without the CA flag keep the response within that limit. The SPSDK tools also reject RSA SRKs and SRKs with the CA flag before contacting the device. This limit may be lifted in a future ELE firmware release.
- With ELE firmware 2.0.5, authentication on the iMX95 only works while the ELE runs from its ROM, that is with the SoC in serial download mode. With the ELE firmware loaded, a valid response is accepted but debug is never opened, and ending the session resets the SoC. This may be an ELE firmware issue, to be fixed in a future release.
- A debug session does not survive a reset (observed on the iMX95 with ELE firmware 2.0.5), although the SoC security reference manual describes a token kept in the battery-backed secure module (BBSM) to reopen debug after a reset.
- The debug credential templates and this flow were validated with SPSDK 3.11.0, the `spsdk-pkcs11` plugin 0.3.8 and ELE firmware 2.0.5.

## References

The implementation was based on the following documents. Access to some of them may be restricted and require a non-disclosure agreement with the SoC vendor.

- AN14579 — *Secure Debug on ELE-AP based i.MX SoCs*, Rev. 1.0, 24 February 2025
- *i.MX 95 Applications Processor Security Reference Manual*, Rev. 4, 2026-01-14
- UG10190 — *EdgeLock Secure Enclave i.MX 95 B0 User Guide (FW version v2.0.3)*, Rev. 1.2, 30 September 2025
