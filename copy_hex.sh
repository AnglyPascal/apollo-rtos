#! /usr/bin/bash

project_root=$PWD
microbit="/run/media/microbit"
binary="$project_root/bare-metal.hex"

copy_hex () {
  device="$(sudo blkid | rg "MICROBIT" | awk '{print substr($1, 1, length($1)-1)}')"
  sudo mount $device $microbit
  sudo cp $binary $microbit
  sudo umount $device
}

copy_hex
