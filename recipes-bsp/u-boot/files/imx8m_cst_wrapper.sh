#!/bin/sh
#
# Wrapper installed as "cst" in the binman tool path. It runs the CST binary
# set in TDX_IMX_HAB_CST_BIN with the additional arguments in
# TDX_IMX_HAB_CST_ARGS.
#
# When LIBFAKETIME_PATH is set, CST runs with a fixed time, so that the CSF
# signatures are the same on every build. The date itself is not important
# (November 6, 2024, 12:00 PM UTC).

if [ -n "${LIBFAKETIME_PATH}" ]; then
    export LD_PRELOAD="${LIBFAKETIME_PATH}${LD_PRELOAD:+ ${LD_PRELOAD}}"
    export FAKETIME_FMT="%s"
    export FAKETIME="@1730894400"
fi

# shellcheck disable=SC2086
exec "${TDX_IMX_HAB_CST_BIN}" ${TDX_IMX_HAB_CST_ARGS} "$@"
