#!/bin/bash

FILE=$1
if [ "$FILE" == "" ]; then
    FILE=uboot_century_nand.env
fi
OUTFILE="${FILE%.*}.scr"

echo "File: '$FILE' -> '$OUTFILE'"

/opt/montage/linux/tools/linux/mkimage -A mips -O linux -T script -C none -a 0 -e 0 -n "'Boot Script'" -d "$FILE" "$OUTFILE"


