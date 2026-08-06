# Kernel Module Signing Development Key

This directory contains an intentionally public and private kernel module
signing development key. It provides reproducible test images only and no
production security.

`MODSIGN_KEYDIR` defaults to this directory. Builds using this default emit a
warning. Use this credential only for local development and CI test images.

Never use this key to sign production modules. Set `MODSIGN_KEYDIR` to a
protected production key directory for production builds:

```bitbake
MODSIGN_KEYDIR = "/secure/path/module-signing-key"
MODSIGN_KEY_NAME = "key-module"
```

The configured directory must contain `${MODSIGN_KEY_NAME}.key` and
`${MODSIGN_KEY_NAME}.crt`. The certificate is embedded in the kernel's
builtin trusted keyring; it is not a UEFI `db` certificate and must not be
enrolled in UEFI.

The configured key files are tracked by kernel task checksums. Replacing a
production key therefore invalidates the kernel build without disabling
sstate.

Private-key scanners must allowlist only this directory. The private key here
is deliberate public test material.
