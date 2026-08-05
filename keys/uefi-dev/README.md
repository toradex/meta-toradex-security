# UEFI Secure Boot Development Keys

This directory contains intentionally public and private UEFI Secure Boot
development keys. They provide reproducible test images only and provide
no production security.

`UEFI_SB_KEYDIR` defaults to this directory. Builds using this default emit a
warning. Use these credentials only for local development and CI test images.

Never use these keys to sign production artifacts. Set `UEFI_SB_KEYDIR` to a
protected production enrollment key directory for production builds. Set
`UEFI_SB_BOOT_KEYDIR` and `UEFI_SB_UKI_KEYDIR` when the leaf signing keys live
elsewhere:

```bitbake
UEFI_SB_KEYDIR = "/secure/path/uefi-production-keys"
UEFI_SB_BOOT_KEYDIR = "/secure/path/uefi-boot-signing-key"
UEFI_SB_UKI_KEYDIR = "/secure/path/uefi-uki-signing-key"
```

The configured directory must contain this complete key set before the build:

```
PK.key   PK.crt   PK.esl   PK.auth
KEK.key  KEK.crt  KEK.esl  KEK.auth
db.esl   db.auth
dbx.esl  dbx.auth
```

The boot signing directory must contain `key-boot.key` and `key-boot.crt`.
The UKI signing directory must contain `key-uki.key` and `key-uki.crt`.
`db.esl` and `db.auth` contain both leaf certificates. Regenerate them with
the KEK whenever either leaf certificate changes.

The signing flow does not generate missing keys. A missing file fails the
build.

The complete key set is tracked by deploy task checksums. Replacing production
keys therefore invalidates signed deploy artifacts without disabling sstate.

Private-key scanners must allowlist only this directory. The private keys here
are deliberate public test credentials.
