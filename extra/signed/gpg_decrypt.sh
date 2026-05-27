#!/bin/bash

images=("SignedFiles/see_bl/see_bl.ubo" "SignedFiles/main/main.ubo" "SignedFiles/logo/logo.ubo" "SignedFiles/upg_onepackage/upg_onepackage.ubo" "SignedFiles/dtb/dtb.ubo" "SignedFiles/roothash/roothash.ubo" "SignedFiles/see/see.ubo")

#images=("db2.bin" "db3.bin" "see_bl.ubo" "main.ubo" "logo.ubo" "upg_onepackage.ubo" "dtb.ubo" "roothash.ubo" "see.ubo" "uboot.ubo")

SRC_DIR=$1
DES_DIR=$2
MIN_VER=$3
VER_ATUAL_SW=$4

rm -fr $DES_DIR
mkdir -p $DES_DIR

for image in "${images[@]}"; do
	cp "$SRC_DIR/${image}.lbs.pgp" $DES_DIR
done

DIR_SIG_DEC="B8_signed_decrypt_${VER_ATUAL_SW}"
rm -fr $DIR_SIG_DEC
mkdir -p $DIR_SIG_DEC

files=("see_bl.ubo" "main.ubo" "logo.ubo" "upg_onepackage.ubo" "dtb.ubo" "roothash.ubo" "see.ubo");
for file in "${files[@]}"; do
	gpg --decrypt --output $DIR_SIG_DEC/$file $DES_DIR/${file}.lbs.pgp
done

echo "Copiando o arquivo rootfs.squashfs para a pasta ${DIR_SIG_DEC}" 
cp B8_*_images/images_upg/rootfs.squashfs $DIR_SIG_DEC

DATA=$(date +%Y%m%d)

PID="6041"
PROD_ID="11f5"
UPDATE_DIR="Update_B8"
OTA_BIN="B8_${DATA}_update_v${VER_ATUAL_SW}.b8"
OTA_BIN_ENC="B8_${DATA}_update_enc_v${VER_ATUAL_SW}.b8"
OTA_TS="${DATA}_ota_century_b8_v${VER_ATUAL_SW}_pid_${PID}_ClaroKu.ts"
SW_VERSION="b8_sw_version"

rm -fr $OTA_BIN
rm -fr $OTA_BIN_ENC
rm -fr $UPDATE_DIR

mkdir $UPDATE_DIR
images=("main.ubo" "roothash.ubo" "rootfs.squashfs")
for image in "${images[@]}"; do
	cp "$DIR_SIG_DEC/${image}" $UPDATE_DIR
done

cd $UPDATE_DIR
sha256sum * > sha256sum.txt
cd -
mksquashfs $UPDATE_DIR $OTA_BIN -comp xz

set -e
KEY=133823896ae71b7b298bd51b15a57059666ef7de62c6fc1f62da3403769e6cb0
SIZE=$(ls -s "$OTA_BIN" | awk '{ print $1 * 1024 }')
fallocate -l "$SIZE" "$OTA_BIN_ENC"
DEVICE=$(sudo losetup -f)
sudo losetup "$DEVICE" "$OTA_BIN_ENC"
DEVSZ=$(sudo blockdev --getsz "$DEVICE")
echo "SZ=$DEVSZ"
echo "DEVICE=$DEVICE"
sudo dmsetup create upgdev --table "0 $DEVSZ crypt aes-xts-plain64 $KEY 0 $DEVICE 0"
sudo dd if="$OTA_BIN" of=/dev/mapper/upgdev
sudo dmsetup remove upgdev
sudo losetup -d "$DEVICE"

rm -fr $OTA_BIN

#MIN_VER_HIGH=$(echo $MIN_VER | awk -F . '{ printf "%02X", $1 }')
#MIN_VER_LOW=$(echo $MIN_VER | awk -F . '{ printf "%02X", $2 }')

ATUAL_VER_HIGH=$(echo $VER_ATUAL_SW | awk -F . '{ printf "%02X", $1 }')
ATUAL_VER_LOW=$(echo $VER_ATUAL_SW | awk -F . '{ printf "%02X", $2 }')

MIN_VER_HIGH=$(echo $MIN_VER | awk -F . '{ printf "%02d", $1 }')
MIN_VER_LOW=$(echo $MIN_VER | awk -F . '{ printf "%02d", $2 }')

ATUAL_VER_HIGH_DEC=$(echo $VER_ATUAL_SW | awk -F . '{ printf "%02d", $1 }')
ATUAL_VER_LOW_DEC=$(echo $VER_ATUAL_SW | awk -F . '{ printf "%02d", $2 }')

echo "Min ${MIN_VER_HIGH} ${MIN_VER_LOW}"
echo "Atual ${ATUAL_VER_HIGH} ${ATUAL_VER_LOW}"

printf "\x${ATUAL_VER_HIGH}\x${ATUAL_VER_LOW}" | dd of=$SW_VERSION  bs=1 count=2
dd if=$OTA_BIN_ENC of=$SW_VERSION seek=2 oflag=seek_bytes conv=notrunc
mv $OTA_BIN_ENC $OTA_BIN_ENC.org
mv $SW_VERSION $OTA_BIN_ENC

#MKTS_CMD="-i ${OTA_BIN_ENC} -o ${OTA_TS} -p ${PID} -m ${MIN_VER} -v ${VER_ATUAL_SW} -h ${PROD_ID}"
MKTS_CMD="-i ${OTA_BIN_ENC} -o ${OTA_TS} -p ${PID} -m ${MIN_VER_HIGH}${MIN_VER_LOW} -v ${ATUAL_VER_HIGH_DEC}${ATUAL_VER_LOW_DEC} -h ${PROD_ID}"
echo $MKTS_CMD
./mkts $MKTS_CMD

rm $OTA_BIN_ENC.org
#rm -rf $DES_DIR
#rm -fr $UPDATE_DIR
echo "Fim script para gerar TS B8"

echo "FIM"

