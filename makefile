# === Compiler Settings ===
CC      = x86_64-elf-gcc
AS      = nasm
LD      = x86_64-elf-ld
CFLAGS  = -ffreestanding -m64 -O0 -Wall -Wextra -mno-sse -mgeneral-regs-only -mno-mmx -mno-avx
LDFLAGS = -T kernel.ld -nostdlib

# === Folder Structure ===
SRC_DIR = .
BUILD_DIR = build

# === Source Files ===
C_SRC = \
    $(SRC_DIR)/UnOSKrnl.c \
	$(SRC_DIR)/idt/idt.c \
    $(SRC_DIR)/idt/isr_handler_c.c \
	$(SRC_DIR)/mem/paging.c \
	$(SRC_DIR)/mem/malloc.c \
	$(SRC_DIR)/graphic/alpaca_gdi.c  \
	$(SRC_DIR)/shceduler/scheduler.c \
	$(SRC_DIR)/shceduler/task.c  \
	$(SRC_DIR)/pic/pic.c \
	$(SRC_DIR)/timer/pit.c \
	$(SRC_DIR)/driver/keyboard.c \
	$(SRC_DIR)/driver/pci/pci.c \
	$(SRC_DIR)/driver/storage/ahci.c \
	$(SRC_DIR)/driver/storage/gpt.c \
	$(SRC_DIR)/driver/storage/fat32.c \
	$(SRC_DIR)/terminal/terminal.c \
	$(SRC_DIR)/terminal/command/echo.c \
	$(SRC_DIR)/terminal/command/party.c \
	$(SRC_DIR)/terminal/command/help.c \
	$(SRC_DIR)/terminal/command/gettick.c \
	$(SRC_DIR)/terminal/command/waitfortick.c \
	$(SRC_DIR)/terminal/command/trybuffer.c \
	$(SRC_DIR)/terminal/command/uptime.c \
	$(SRC_DIR)/terminal/command/parsefat32.c \
	$(SRC_DIR)/string/string.c \
	$(SRC_DIR)/filesystem/vfs.c

ASM_SRC = \
	$(SRC_DIR)/boot/boot.asm \
    $(SRC_DIR)/idt/isr_stub.asm  \
	$(SRC_DIR)/boot/paging.asm
    

# === Object Files ===
C_OBJ = $(C_SRC:%.c=$(BUILD_DIR)/%.o)
ASM_OBJ = $(ASM_SRC:%.asm=$(BUILD_DIR)/%.o)
OBJ = $(C_OBJ) $(ASM_OBJ)

# === Output File ===
KERNEL_ELF = $(BUILD_DIR)/UnOSKrnl.elf

# === Default Target ===
all: $(KERNEL_ELF)

# === Rule: Link ELF ===
$(KERNEL_ELF): $(OBJ)
	@mkdir -p $(dir $@)
	$(LD) $(OBJ) $(LDFLAGS) -o $@
	@echo "[LD] $@ done."

# === Rule: Compile C ===
$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@
	@echo "[CC] $< -> $@"

# === Rule: Assemble ASM ===
$(BUILD_DIR)/%.o: %.asm
	@mkdir -p $(dir $@)
	$(AS) -f elf64 $< -o $@
	@echo "[AS] $< -> $@"

# === Clean Target ===
clean:
	rm -rf $(BUILD_DIR)

# === Run in QEMU (Opsional) ===
run: all
	qemu-system-x86_64 -kernel $(KERNEL_ELF)

.PHONY: all clean run
