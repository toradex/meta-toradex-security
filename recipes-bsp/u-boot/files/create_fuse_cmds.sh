#!/bin/bash

set -e

# parameters
SOC="$1"
SRK_FUSE_FILE="$2"
FUSE_CMDS_FILE="$3"

# warning to program SRK hash fuses
WARNING1="\
# These are One-Time Programmable e-fuses. Once you write them you can't
# go back, so get it right the first time!"

# warning to 'close' the device
WARNING2="\
# After the device successfully boots a signed image without generating
# any HAB events, it is safe to secure, or 'close', the device. This is
# the last step in the process. Once the fuse is blown, the chip does
# not load an image that has not been signed using the correct PKI tree.
# Be careful! This is again a One-Time Programmable e-fuse. Once you
# write it you can't go back, so get it right the first time. If
# anything in the previous steps wasn't done correctly, after writing
# this bit, the SOM will not boot anymore!"

fuse_write_line() {
    bank=$1
    word=$2
    hexval=$3
    echo "fuse prog -y $bank $word $hexval"
}

create_fuse_cmds_mx6() {
    echo "${WARNING1}"

    # commands to program SRK_HASH fuses
    bank=3
    word=0
    for hexval in $(hexdump -e '/4 "0x"' -e '/4 "%X""\n"' ${SRK_FUSE_FILE}); do
        fuse_write_line $bank $word $hexval
        word=$((word+1))
    done

    # command to program SEC_CONFIG fuse and 'close' the device
    echo -e "\n${WARNING2}"
    fuse_write_line 0 6 0x00000002
}

case ${SOC} in
    "IMX6ULL")
        create_fuse_cmds_mx6 > ${FUSE_CMDS_FILE}
        ;;
    *)
        echo "Invalid SOC!"
        return 1
        ;;
esac
