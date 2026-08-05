# UEFI Secure Boot on Intel x86-64

This document describes the `tdx-signed` UEFI Secure Boot configuration for
`MACHINE = "intel-corei7-64"`.

## Scope

This configuration applies only to the signed x86 image. It does not use the
U-Boot hardening or FIT image signing flows described for NXP and TI platforms.

The signed boot chain is:

```
UEFI firmware -> systemd-boot -> ext4_x64.efi -> UKI -> OSTree deployment
```

`systemd-boot`, the ext4 UEFI driver, and the Unified Kernel Image (UKI) are
signed with the `db` key. The UEFI firmware accepts them only after the
corresponding PK, KEK, and db keys have been enrolled.

## Build configuration

Enable the signed configuration in the build configuration:

```bitbake
MACHINE = "intel-corei7-64"
INHERIT += "tdx-signed"
```

Build the required image normally, for example:

```sh
bitbake torizon-docker
```

The signed configuration selects `systemd-boot`, enables the OVMF Secure Boot
and TPM 2.0 build options, and produces the signed WIC layout.

## Image layout

The signed WIC image contains these GPT partitions:

| Mount point | Label | Filesystem | Purpose |
| --- | --- | --- | --- |
| `/var/rootdirs/media/efi` | `efi` | FAT32 | EFI System Partition containing systemd-boot, enrollment artifacts, and the signed ext4 driver |
| `/boot` | `xbootldr` | ext4 | XBOOTLDR partition containing OSTree boot artifacts and UKIs |
| `/` | `otaroot` | ext4 | OSTree root filesystem |

The signed layout differs from the unsigned GRUB image. Existing devices need a
reflash to adopt this partition layout.

## UEFI keys

`UEFI_SB_KEYDIR` defaults to the tracked public development keys in
`meta-toradex-security/keys/uefi-dev`. The build warns when these keys are in
use. They are suitable only for local development and CI test images.

For production, set `UEFI_SB_KEYDIR` to a protected directory containing the
externally managed PK, KEK, db, and dbx enrollment material. Set
`UEFI_SB_BOOT_KEYDIR` and `UEFI_SB_UKI_KEYDIR` when signing keys are stored
separately. The boot key signs systemd-boot and the ext4 driver; the UKI key
signs UKIs. The enrolled `db` variable must contain both leaf certificates.
The signing flow does not generate missing key material. See
[`../keys/uefi-dev/README.md`](../keys/uefi-dev/README.md) for required files.

`UEFI_SB_SIGN_ENABLE` defaults to `1`. Setting it to `0` leaves EFI binaries
unsigned and is not a Secure Boot configuration.

## Initial enrollment

The EFI System Partition contains PK, KEK, db, and dbx enrollment artifacts
under `loader/keys/auto/`. These artifacts are for initial firmware enrollment;
they must not be used for OTA updates.

The build also creates an OVMF Secure Boot variable-store artifact with PK,
KEK, and db enrolled. Use an OVMF variable store created by this build for VM
testing. Its mutable working copy must persist between VM sessions when testing
firmware-variable changes.

## VM validation

Boot the signed image with Secure Boot enabled and verify in the guest:

```sh
mokutil --sb-state
bootctl status
ostree admin status
tpm2_getcap properties-fixed
```

`mokutil --sb-state` must report Secure Boot enabled and `bootctl status` must
identify systemd-boot as active.

## OTA UEFI variable updates

The signed image supports append-only DB and DBX updates through Torizon
Subsystem Updates. Configure build inputs with `UEFI_SB_DB_ADD_CERTS` and
`UEFI_SB_DBX_ADD_HASHES` or `UEFI_SB_DBX_ADD_CERTS`; generated `.auth` payloads
and JSON manifests are written to `${DEPLOYDIR}/uefi-sb-updates`.

DB and DBX update artifacts are distinct from initial enrollment files. DB key
removal, PK/KEK replacement, and automatic rollback of UEFI variable changes
are unsupported. Never revoke a binary or signer needed by the active boot
chain. See the UEFI OTA update runbook for deployment and verification steps.

## Design notes

The signed configuration replaces GRUB with systemd-boot so firmware loads
signed EFI binaries directly. The UKI provides a signed kernel, initramfs, and
kernel command line. Deployment-specific OSTree selection uses systemd-boot
boot entries. Document the `LoaderEntrySelected` resolution path only when the
corresponding initramfs integration is enabled.
