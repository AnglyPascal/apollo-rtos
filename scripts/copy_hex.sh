#!/bin/bash

# Ensure an argument is provided
if [ $# -ne 1 ]; then
    echo "Usage: $0 <hex_file>"
    exit 1
fi

# Set variables
project_root=$PWD/build/
hex_file="$project_root/$1.hex"
microbit="/run/media/microbit"

# Function to copy HEX file
copy_hex () {
    device="$(sudo blkid | rg 'MICROBIT' | awk '{print substr($1, 1, length($1)-1)}')"

    if [ -z "$device" ]; then
        echo "Error: No MICROBIT device found!"
        exit 1
    fi

    # Check if the HEX file exists
    if [ ! -f "$hex_file" ]; then
        echo "Error: File '$hex_file' not found!"
        exit 1
    fi

    # Mount, copy, and unmount
    sudo mount $device $microbit
    sudo cp "$hex_file" "$microbit"
    sudo umount $device
}

copy_hex
