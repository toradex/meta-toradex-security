# UEFI DB and DBX OTA Updates on Intel x86-64

This runbook applies append-only UEFI Secure Boot variable updates to signed
`intel-corei7-64` images through Torizon Subsystem Updates.

It covers adding a db signer, adding a dbx Authenticode SHA-256 revocation,
and revoking a db signer through dbx.
It does not support db key removal, PK or KEK replacement, or rollback of a
firmware-variable update.

## Prerequisites

- Build the signed image with `INHERIT += "tdx-signed"`.
- Use UEFI enrollment material through `UEFI_SB_KEYDIR` and the boot and UKI
  signing keys through `UEFI_SB_BOOT_KEYDIR` and `UEFI_SB_UKI_KEYDIR`.
- Enroll PK and KEK before applying an update. The device must be in UEFI User
  Mode.
- Use Torizon Cloud with Subsystem Updates enabled and a provisioned device.
- Do not use initial enrollment files from `loader/keys/auto/` as OTA payloads.

The image registers these subsystem-update hardware IDs:

| Hardware ID | UEFI variable | Build input |
| --- | --- | --- |
| `uefi-db-append` | `db` | `UEFI_SB_DB_ADD_CERTS` |
| `uefi-dbx-append` | `dbx` | `UEFI_SB_DBX_ADD_HASHES`, `UEFI_SB_DBX_ADD_CERTS` |

## Build update artifacts

Set one or both inputs in the build configuration:

```bitbake
UEFI_SB_DB_ADD_CERTS = "/secure/keys/db-next.crt"
UEFI_SB_DBX_ADD_HASHES = "<64-character-authenticode-sha256>"
UEFI_SB_DBX_ADD_CERTS = "/secure/keys/db-old.crt"
```

The DB input is a PEM X.509 certificate. Each DBX input is a 64-character
hexadecimal Authenticode SHA-256 digest or a PEM X.509 certificate. The hash
input creates an `EFI_CERT_SHA256` entry that revokes one PE binary. The
certificate input creates an `EFI_CERT_X509` entry that revokes every binary
signed by that certificate. Build the image or run the deploy task:

```sh
bitbake torizon-docker
```

Artifacts and their manifests are written to:

```text
${DEPLOY_DIR_IMAGE}/uefi-sb-updates/
```

Artifact names are deterministic:

```text
db-append-${DISTRO_VERSION}-<certificate-sha256>.auth
dbx-append-${DISTRO_VERSION}-<authenticode-sha256>.auth
dbx-cert-append-${DISTRO_VERSION}-<certificate-sha256>.auth
```

Each `.auth` file is signed by `KEK.key` and `KEK.crt` with append semantics.
Its adjacent JSON manifest supplies the hardware ID, package version, source
digest, and payload digest.

When migrating from a single signing key to separate boot and UKI keys, append
the UKI signing certificate through `UEFI_SB_DB_ADD_CERTS` before deploying an
image signed by that key. Otherwise firmware on already-enrolled devices will
not trust the new UKI.

## Publish and assign

Upload one artifact using its manifest values. For a DBX update:

```sh
torizoncore-builder platform push \
    --credentials credentials.zip \
    --hardwareid uefi-dbx-append \
    --package-name <manifest-derived-name> \
    --package-version <build-version> \
    --custom-meta "$(cat <artifact>.auth.json)" \
    <artifact>.auth
```

Use `uefi-db-append` for a DB artifact. Assign the uploaded package to a test
device before wider deployment.

## Apply and verify

During installation, aktualizr verifies the payload checksum and invokes the
UEFI update handler. The handler rejects updates when PK or KEK is absent,
firmware rejects the KEK-signed payload, or the selected variable is unchanged.

The handler reports `need-completion` and requests reboot after a successful
write. After reboot, it confirms that the target UEFI variable remains readable.
Verify the result on the device:

```sh
efi-readvar -v db
efi-readvar -v dbx
```

For a DBX test, attempt to boot only a disposable, known-bad test binary whose
hash was added to DBX. Firmware must reject it.

## DB signer rotation and revocation

1. Create a new UKI signing key and certificate. Do not regenerate PK or KEK.
2. Build R1 with the old UKI key and set `UEFI_SB_DB_ADD_CERTS` to the new
   certificate. Publish the OS update and then apply its DB append artifact as
   a separate update campaign.
3. Verify that every target device accepts the new certificate in `db`.
4. Build and deploy R2 with the new UKI key. Keep the old certificate in `db`
   during the rollout and soak period.
5. If the old key requires revocation, build a DBX artifact with
   `UEFI_SB_DBX_ADD_CERTS` set to the old certificate, then apply it to a
   canary device before fleet rollout.

The build rejects revoking either certificate currently configured to sign the
bootloader or UKI. Build the revocation artifact with the new UKI key active.
Applying the DBX artifact prevents booting any prior deployment signed by the
old key, including OSTree rollback deployments.

## DBX safety rules

- Never revoke the active UKI, systemd-boot, ext4 UEFI driver, or their signer.
- Validate candidate DBX hashes in an isolated VM before publication.
- Validate a certificate revocation against both a new-key UKI that must boot
  and an old-key UKI that must be rejected.
- Start with a single test device and retain recovery access before rollout.
- UEFI variable updates cannot be automatically rolled back. A rejected or
  unsafe DBX entry can prevent later boots.
