#!/bin/bash 
qemu-system-x86_64  -smp 4 -m 4G -cpu qemu64 -kernel x32/build/kernel32  -device loader,file="x64/build/kernel64" -serial stdio -nographic -monitor null -s -S 
