Fuse Template Files
===================

This directory contains template files consumed by create_fuse_cmds.sh to
generate the per-board fuse-programming command file (fuse-cmds.txt) and the
human-readable fuse map (imx-config.fuse).

There are two template flavors:

  *-template.fuse           HAB/AHAB SRK-hash + boot-close template
                            (one per SoC family)

  *-sjc-template.fuse       SJC (Secure JTAG Controller) fuse map for
                            the Secure Debug feature (one per SoC family)

The script chooses the right template from the SOC positional argument.


HAB/AHAB template format
------------------------

Each non-empty line is colon-separated. The first field is a tag:

  H:T:<type>                Header. <type> is HAB or AHAB. It tells the
                            script (and the reader) what device feature the
                            template targets. The script also uses the
                            presence of "H:T:HAB" to decide whether the
                            close step emits an explicit fuse write or the
                            'ahab_close' u-boot command (no H:T:HAB -> AHAB).

  H:F:<bank>:<word>:        SRK-hash fuse slot. The trailing field is left
                            empty; create_fuse_cmds.sh fills it at runtime,
                            consuming one 32-bit word at a time from the
                            SRK fuse binary (TDX_IMX_HAB_CST_SRK_FUSE).
                            The number of H:F lines must match the size of
                            that binary, in 32-bit words.

  H:C:<bank>:<word>:<hex>   HAB-only "close" fuse write. Programmed last,
                            after the SRK-hash lines. AHAB targets omit
                            this line; the script emits 'ahab_close' instead.


SJC template format
-------------------

Used by the Secure Debug feature to drive secure_debug_append() in
create_fuse_cmds.sh. Format:

  H:T:SJC                       Header (informational).

  SJC:<name>:<bank>:<word>:<mask>
                                One row per writable fuse field.
                                <name>  symbolic identifier referenced by
                                        the script (e.g. SJC_RESP_LO,
                                        JTAG_SMODE_SECURE, SJC_DISABLE).
                                <mask>  fixed hex value written to that
                                        bank:word. An empty <mask> means
                                        the value is supplied at runtime.

The set of symbolic names the script expects is fixed by
secure_debug_append() in create_fuse_cmds.sh. Adding a new SoC means
providing the same names with the bank/word/mask values for that SoC's
fuse map; do not invent new names without also updating the script.

The order of SJC: rows in the file is irrelevant -- the script looks
them up by name.
