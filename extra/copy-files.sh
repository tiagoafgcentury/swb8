#!/bin/bash

DEST=$1
ORIG=`dirname $0`
SRC=/opt/montage/linux/image/alaska

if [ ! -d "$DEST" ]; then
	echo "$DEST must be a directory"
	exit 1
fi

for FILE in `awk '{ if ($2 == "fatload") print $NF }' "$ORIG/flash-montage.txt"`; do
	if [ -f "$SRC/$FILE" ]; then
		cp -v "$SRC/$FILE" "$DEST"
	else
		find "$SRC" -type f -name "$FILE" -exec cp -v {} "$DEST" \;
	fi
done
