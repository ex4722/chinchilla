export CURRENT_DIR := $(shell pwd)

export INCLUDE_DIR = $(CURRENT_DIR)/include
export SRC_DIR := $(CURRENT_DIR)/src
export BUILD_DIR := $(CURRENT_DIR)/build
export CFLAGS = -g -I $(INCLUDE_DIR) -c -ffreestanding
export LDFLAGS = -T $(CURRENT_DIR)/linker.ld -std=gnu99 -O2 -Wall -Wextra -nostdlib -m64 -fno-pie -lgcc
export ASMFLAGS = -felf64
export CC:=$(CC64)




kernel64: src | $(BUILD_DIR)
	$(CC) $(LDFLAGS) -o $@ $(shell find $(BUILD_DIR) -name '*.o')

.PHONY: src
src:
	make -C src

.PHONY: clean
clean:
	rm -rf $(BUILD_DIR)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)
