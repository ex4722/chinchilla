# @TODO: can't just append to BUILD_DIR due to double appending issues
OUT_DIR := $(addsuffix /utils, $(BUILD_DIR))

TARGETS := printf string helper acpi
TARGETS := $(addprefix $(OUT_DIR)/, $(addsuffix .o, $(TARGETS)))


all:  
	make $(TARGETS) 
$(OUT_DIR):
	mkdir -p $(OUT_DIR)
$(OUT_DIR)/printf.o: printf.c | $(OUT_DIR)
	$(CC) $(CFLAGS) -o $@ $^
$(OUT_DIR)/string.o: string.c | $(OUT_DIR)
	$(CC) $(CFLAGS) -o $@ $^
$(OUT_DIR)/helper.o: helper.s | $(OUT_DIR)
	$(ASM) $(ASMFLAGS) -o $@ $^
$(OUT_DIR)/acpi.o: acpi.c | $(OUT_DIR)
	$(CC) $(CFLAGS) -o $@ $^
