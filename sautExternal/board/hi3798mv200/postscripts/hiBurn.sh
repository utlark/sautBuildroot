#!/usr/bin/env bash

IMAGE_DIR="$1"
BOARD_DIR="$BR2_EXTERNAL_SAUT_PATH/board/hi3798mv200"

cp -r $BOARD_DIR/postscripts/emmc_saut $IMAGE_DIR/
cp    $IMAGE_DIR/uImage.hi3798mv200    $IMAGE_DIR/emmc_saut/kernel.bin
cp    $IMAGE_DIR/rootfs.ext2           $IMAGE_DIR/emmc_saut/rootfs.ext4

$BOARD_DIR/postscripts/mkbootargs -s 64 -r $BOARD_DIR/bootargs.txt -o $IMAGE_DIR/emmc_saut/bootargs.bin
$BOARD_DIR/postscripts/resize2fs -M $IMAGE_DIR/emmc_saut/rootfs.ext4
