#!/bin/bash

set -e

BASEDIR=$(dirname "$0")
BASEDIR=$(realpath "$BASEDIR")

images=("see_bl.ubo" "main.ubo" "logo.ubo" "upg_onepackage.ubo" "dtb.ubo" "roothash.ubo" "see.ubo")

SRC_DIR=$1
DES_DIR=$2
DES_DIR_16=out_16ByteAlign
NAGRA_KEY=NASC_signing@nagra.com

rm -fr $DES_DIR
mkdir -p $DES_DIR
for image in "${images[@]}"; do
	cp $SRC_DIR/$image $DES_DIR
done

m=16
rm -fr $DES_DIR_16
mkdir -p $DES_DIR_16
for image in "${images[@]}"; do
	echo $DES_DIR/$image
	filesize="$(stat -c "%s" "$DES_DIR/$image")"
	echo $filesize
	echo $DES_DIR/$image
	echo $DES_DIR_16/$image
	echo dd if=$DES_DIR/$image of=$DES_DIR_16/$image bs=$filesize count=1
	dd if=$DES_DIR/$image of=$DES_DIR_16/$image bs=$filesize count=1
	result=$(echo "$filesize % $m" | bc)
	echo "result:"$result	
	if [ $result -ne 0 ];then
		dd if=/dev/zero of=$DES_DIR_16/$image seek=$filesize oflag=seek_bytes bs=$(echo "$m - $result" | bc) count=1 
	fi
done

cd $DES_DIR_16
for image in "${images[@]}"; do
	gpg -e -r $NAGRA_KEY $image 
done
