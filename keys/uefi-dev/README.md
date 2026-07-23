# UEFI Secure Boot Development Keys

This directory contains intentionally public and private UEFI Secure Boot
development keys. They provide reproducible test images only and provide
no production security.

Never use these keys to sign production artifacts. Set `UEFI_SB_KEYDIR` to a
protected production key directory for production builds:

```bitbake
UEFI_SB_KEYDIR = "/secure/path/uefi-production-keys"
```

The complete key set is tracked by deploy task checksums. Replacing production
keys therefore invalidates signed deploy artifacts without disabling sstate.

Private-key scanners must allowlist only this directory. The private keys here
are deliberate public test credentials.
