FROM ubuntu:24.04

RUN apt update -y
RUN apt upgrade -y
RUN apt install wget unzip -y

RUN mkdir -p /toolchain/i686 /toolchain/x86_64

WORKDIR /toolchain/i686

RUN wget https://github.com/lordmilko/i686-elf-tools/releases/download/13.2.0/i686-elf-tools-linux.zip
RUN unzip i686-elf-tools-linux.zip && rm i686-elf-tools-linux.zip

WORKDIR /toolchain/x86_64

RUN wget https://github.com/lordmilko/i686-elf-tools/releases/download/13.2.0/x86_64-elf-tools-linux.zip
RUN unzip x86_64-elf-tools-linux.zip && rm x86_64-elf-tools-linux.zip

# Always assumed that working dir is mounted when docker is run @ /chinchilla
