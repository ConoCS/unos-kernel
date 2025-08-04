# === Root Directory Structure ===
ROOT_DIR := .
BUILD_DIR := build
INCLUDE_DIR := Include

# === Compiler & Flags ===
CC := x86_64-elf-gcc
AS := nasm
LD := x86_64-elf-ld

CFLAGS := -ffreestanding -O0 -Wall -Wextra -mno-sse2 -mno-sse -mno-mmx -mno-3dnow \
          -fno-tree-vectorize -mno-80387 -I$(INCLUDE_DIR) -I.

# === Source Files ===
C_SRC := $(shell find $(ROOT_DIR) -type f -name "*.c")
ASM_SRC := $(shell find $(ROOT_DIR) -type f -name "*.asm")

# === Object Files (preserve folder structure in build) ===
C_OBJ := $(patsubst $(ROOT_DIR)/%.c, $(BUILD_DIR)/%.o, $(C_SRC))
ASM_OBJ := $(patsubst $(ROOT_DIR)/%.asm, $(BUILD_DIR)/%.o, $(ASM_SRC))
OBJ := $(C_OBJ) $(ASM_OBJ)

# === Output ===
KERNEL := $(BUILD_DIR)/UnOSKrnl.elf

# === Linker Script ===
LINKER_SCRIPT := arch/common/kernel.ld

# === Rules ===
.PHONY: all clean

all: $(KERNEL)

$(KERNEL): $(OBJ)
	@echo "== Linking kernel ELF =="
	$(LD) -n -T $(LINKER_SCRIPT) -o $@ $^

# Rule for C source
$(BUILD_DIR)/%.o: $(ROOT_DIR)/%.c
	@mkdir -p $(dir $@)
	@echo "== Compiling $< =="
	$(CC) $(CFLAGS) -c $< -o $@

# Rule for ASM source
$(BUILD_DIR)/%.o: $(ROOT_DIR)/%.asm
	@mkdir -p $(dir $@)
	@echo "== Assembling $< =="
	$(AS) -f elf64 $< -o $@

clean:
	rm -rf $(BUILD_DIR)
