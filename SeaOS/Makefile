# SeaOS – Minimal Proof-of-Concept Build

AS       = nasm
CC       = i686-elf-gcc
LD       = i686-elf-ld
OBJCOPY  = i686-elf-objcopy
CFLAGS   = -m32 -ffreestanding -nostdlib -O2 -Wall -Wextra
LDFLAGS  = -m elf_i386 -T kernel/link.ld

BUILD_DIR := build
BOOT_BIN  := $(BUILD_DIR)/boot.bin
KERNEL_ELF:= $(BUILD_DIR)/kernel.elf
KERNEL_BIN:= $(BUILD_DIR)/kernel.bin
OS_IMG    := $(BUILD_DIR)/SeaOS.img

all: $(OS_IMG)

$(BUILD_DIR):
	@mkdir -p $@

$(BOOT_BIN): | $(BUILD_DIR)
	$(AS) SeaOS/boot/boot.asm -f bin -o $@

$(KERNEL_ELF): SeaOS/kernel/kernel.c SeaOS/kernel/link.ld | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $(BUILD_DIR)/kernel.o
	$(LD) $(LDFLAGS) -o $@ $(BUILD_DIR)/kernel.o

$(KERNEL_BIN): $(KERNEL_ELF)
	$(OBJCOPY) -O binary $< $@

$(OS_IMG): $(BOOT_BIN) $(KERNEL_BIN)
	cat $^ > $@

run: $(OS_IMG)
	qemu-system-i386 -drive format=raw,file=$< -serial mon:stdio

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all clean run