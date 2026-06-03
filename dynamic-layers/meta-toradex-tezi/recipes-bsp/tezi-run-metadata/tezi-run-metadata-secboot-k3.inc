# When targeting a fused HS-SE device, the Tezi recovery scripts must
# DFU-load the HS-SE signed tiboot3 variant. The upstream scripts
# hardcode the HS-FS variant (tiboot3-<soc>-hs-fs-<board>.bin), which
# the boot ROM of a fused device rejects. To fix this, rewrite the
# tiboot3 filename from its HS-FS to its HS-SE variant in both the Linux
# and Windows recovery scripts.
do_deploy:append() {
    if [ "${SYSFW_SUFFIX}" = "hs" ]; then
        for f in recovery-linux.sh recovery-windows.bat; do
            script="${TEZI_RUN_DEPLOYDIR}/$f"
            if ! grep -q -E 'tiboot3-[[:alnum:]]+-hs-fs-[[:alnum:]]+\.bin' "$script"; then
                bbfatal "Expected HS-FS tiboot3 filename not found in $f." \
                    "The upstream recovery script format may have changed!"
            fi
            sed -i -E 's/(tiboot3-[[:alnum:]]+-)hs-fs(-[[:alnum:]]+\.bin)/\1hs\2/g' "$script"
        done
    fi
}
