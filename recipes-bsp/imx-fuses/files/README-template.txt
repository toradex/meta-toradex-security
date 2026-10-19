Fuse Template Files
===================

This directory contains template files consumed by create_fuse_cmds.sh to
generate the per-board fuse-programming command file (fuse-cmds.txt) and the
human-readable fuse map (imx-config.fuse).

There are two template flavors:

  *-template.fuse           HAB/AHAB SRK-hash + boot-close template
                            (one per SoC family)

  *-sjc-template.fuse       SJC (System JTAG Controller) fuse map for
                            the Secure Debug feature (one per SoC family)

  *-ele-template.fuse       EdgeLock Secure Enclave (ELE) debug fuse map
                            for the Secure Debug feature (one per SoC)

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
secure_debug_append() in create_fuse_cmds.sh, and differs between the
i.MX6/i.MX7/i.MX8M and the i.MX8/i.MX8X families, which have different
debug fuses. Adding a new SoC means providing the names of its family
with the bank/word/mask values for that SoC's fuse map; do not invent
new names without also updating the script.

The order of SJC: rows in the file is irrelevant -- the script looks
them up by name. Lines that do not start with "SJC:" are ignored, so a
template may carry "#" comments explaining where its values come from.

A template only needs the rows that the modes supported on its SoC can
reach. Leaving a row out is a deliberate safety net: create_fuse_cmds.sh
aborts on a missing entry rather than programming a fuse whose layout it
cannot know.

As it emits each fuse command, the script also appends the resolved row
(same format, with <mask> replaced by the value actually programmed) to
imx-config.fuse, so that file records every fuse the commands burn.

ELE template format
-------------------

Used by the Secure Debug feature on SoCs where debug access is controlled
by the EdgeLock Secure Enclave (iMX9x). Format:

  H:T:ELE                       Header. Selects the ELE flow in
                                secure_debug_append().

  ELE:<name>:<bank>:<word>:<mask>
                                Same row format as the SJC template, with
                                a fixed <mask>.

Recognized names:

  DBG_DISABLE_<domain>          Debug-disable bits of one debug domain
                                (the 4 CoreSight enables of the domain).
                                The script burns every DBG_DISABLE_* row it
                                finds, so the rows list the domains of the
                                SoC; <domain> is only used in comments. Rows
                                sharing a fuse word are merged into a single
                                write.

  JTAG_DISABLE                  Fuse that disables the JTAG controller,
                                boundary scan included (named SJC_DISABLE in
                                the NXP documentation).

Authenticated debug on these SoCs needs no fuses, so a template only
describes the fuses used by the "no-debug" mode and the full JTAG disable.
