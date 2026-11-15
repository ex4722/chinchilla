export SHELL = /bin/sh

.SUFFIXES:
.SUFFIXES: .c .o .s


export CC32:=i686-elf-gcc
export CC64:=x86_64-elf-gcc
export ASM=nasm 


all:
	make -C x32 
	make -C x64
clean:
	make -C x32  clean
	make -C x64  clean
run:
	bash start.sh
