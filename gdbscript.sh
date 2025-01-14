#! /usr/bin/bash

tmux new-session -d -s debug "qemu-system-arm -machine microbit -kernel bare-metal.elf -S -gdb tcp::1234 -serial stdio"
tmux split-window -h "arm-none-eabi-gdb bare-metal.elf -ex 'target remote :1234'"
tmux attach-session -t debug
