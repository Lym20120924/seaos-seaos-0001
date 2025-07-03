# SeaOS Kernel Makefile
# Supports PC, PE, Tablet, and Mobile platforms with AI system

# Compiler and tools
CC = gcc
LD = ld
AS = nasm
OBJCOPY = objcopy

# Platform selection (default: PC)
PLATFORM ?= PC

# Architecture selection
ARCH ?= x86_64

# Compiler flags
CFLAGS = -m64 -ffreestanding -fno-stack-protector -fno-pic -mno-red-zone
CFLAGS += -Wall -Wextra -Werror -std=c11
CFLAGS += -Ikernel/include -Ikernel/core -Ikernel/ai
CFLAGS += -DPLATFORM_$(PLATFORM)_BUILD

# Linker flags
LDFLAGS = -T kernel.ld -nostdlib -z max-page-size=0x1000

# Platform-specific flags
ifeq ($(PLATFORM),PC)
    CFLAGS += -DPLATFORM_PC
    PLATFORM_SOURCES = kernel/platform/pc/pc_init.c \
                      kernel/platform/pc/pc_acpi.c \
                      kernel/platform/pc/pc_pci.c \
                      kernel/platform/pc/pc_smp.c
endif

ifeq ($(PLATFORM),PE)
    CFLAGS += -DPLATFORM_PE
    PLATFORM_SOURCES = kernel/platform/pe/pe_init.c \
                      kernel/platform/pe/pe_uefi.c \
                      kernel/platform/pe/pe_secure_boot.c \
                      kernel/platform/pe/pe_power.c
endif

ifeq ($(PLATFORM),TABLET)
    CFLAGS += -DPLATFORM_TABLET
    PLATFORM_SOURCES = kernel/platform/tablet/tablet_init.c \
                      kernel/platform/tablet/tablet_sensors.c \
                      kernel/platform/tablet/tablet_power.c \
                      kernel/platform/tablet/tablet_display.c \
                      kernel/platform/tablet/tablet_audio.c
endif

ifeq ($(PLATFORM),MOBILE)
    CFLAGS += -DPLATFORM_MOBILE
    PLATFORM_SOURCES = kernel/platform/mobile/mobile_init.c \
                      kernel/platform/mobile/mobile_telephony.c \
                      kernel/platform/mobile/mobile_sensors.c \
                      kernel/platform/mobile/mobile_power.c \
                      kernel/platform/mobile/mobile_display.c \
                      kernel/platform/mobile/mobile_audio.c \
                      kernel/platform/mobile/mobile_camera.c \
                      kernel/platform/mobile/mobile_gps.c
endif

# Source files
CORE_SOURCES = kernel/core/main.c \
               kernel/core/kprintf.c \
               kernel/core/mm.c \
               kernel/core/process.c \
               kernel/core/syscall.c \
               kernel/core/panic.c

AI_SOURCES = kernel/ai/ai_core.c \
             kernel/ai/ai_device_detection.c \
             kernel/ai/ai_package_manager.c \
             kernel/ai/ai_neural_network.c

DRIVER_SOURCES = kernel/drivers/gpu/vga.c \
                kernel/drivers/gpu/vulkan.c \
                kernel/drivers/gpu/metal.m \
                kernel/drivers/input/ps2.c \
                kernel/drivers/input/touch.c \
                kernel/drivers/net/rtl8139.c \
                kernel/drivers/net/wifi.c

NET_SOURCES = kernel/net/ip.c \
              kernel/net/tcp.c \
              kernel/net/udp.c \
              kernel/net/icmp.c \
              kernel/net/arp.c \
              kernel/net/dhcp.c \
              kernel/net/dns.c

FS_SOURCES = kernel/fs/ext4.c \
             kernel/fs/ntfs.c

BOOT_SOURCES = boot/efi/bootx64.c

# All sources
SOURCES = $(CORE_SOURCES) $(AI_SOURCES) $(DRIVER_SOURCES) $(NET_SOURCES) $(FS_SOURCES) $(PLATFORM_SOURCES)

# Object files
OBJECTS = $(SOURCES:.c=.o)
OBJECTS := $(OBJECTS:.m=.o)

# Boot objects
BOOT_OBJECTS = $(BOOT_SOURCES:.c=.o)

# Targets
.PHONY: all clean pc pe tablet mobile

all: seaos-$(PLATFORM).iso

pc:
	$(MAKE) PLATFORM=PC

pe:
	$(MAKE) PLATFORM=PE

tablet:
	$(MAKE) PLATFORM=TABLET

mobile:
	$(MAKE) PLATFORM=MOBILE

# Kernel binary
kernel.bin: $(OBJECTS)
	$(LD) $(LDFLAGS) -o $@ $^

# Bootloader
bootx64.efi: $(BOOT_OBJECTS)
	$(LD) -shared -Bsymbolic -T boot/efi/efi.lds -o $@ $^

# ISO image
seaos-$(PLATFORM).iso: kernel.bin bootx64.efi
	mkdir -p iso/EFI/BOOT
	cp bootx64.efi iso/EFI/BOOT/
	cp kernel.bin iso/
	genisoimage -o $@ -b EFI/BOOT/bootx64.efi -no-emul-boot -boot-load-size 4 -boot-info-table iso/

# Object file compilation
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.m
	$(CC) $(CFLAGS) -c $< -o $@

# Clean
clean:
	rm -f $(OBJECTS) $(BOOT_OBJECTS)
	rm -f kernel.bin bootx64.efi
	rm -f seaos-*.iso
	rm -rf iso/

# Install targets
install-pc: seaos-PC.iso
	@echo "Installing SeaOS for PC platform"
	# Copy to USB or burn to CD/DVD

install-pe: seaos-PE.iso
	@echo "Installing SeaOS PE environment"
	# Deploy to PXE server or create bootable USB

install-tablet: seaos-TABLET.iso
	@echo "Installing SeaOS for tablet platform"
	# Flash to tablet storage or create recovery image

install-mobile: seaos-MOBILE.iso
	@echo "Installing SeaOS for mobile platform"
	# Flash to mobile device or create recovery image

# Debug targets
debug-pc:
	qemu-system-x86_64 -cdrom seaos-PC.iso -m 512M -enable-kvm

debug-pe:
	qemu-system-x86_64 -cdrom seaos-PE.iso -m 256M -netdev user,id=net0 -device e1000,netdev=net0

debug-tablet:
	qemu-system-x86_64 -cdrom seaos-TABLET.iso -m 1G -device usb-tablet

debug-mobile:
	qemu-system-x86_64 -cdrom seaos-MOBILE.iso -m 1G -device usb-tablet -netdev user,id=net0 -device e1000,netdev=net0

# Package creation targets
create-exe: seaos-PC.iso
	@echo "Creating Windows EXE installer"
	# TODO: Create Windows installer package

create-apk: seaos-MOBILE.iso
	@echo "Creating Android APK package"
	# TODO: Create Android package

create-ipa: seaos-MOBILE.iso
	@echo "Creating iOS IPA package"
	# TODO: Create iOS package

create-deb: seaos-PC.iso
	@echo "Creating Debian DEB package"
	# TODO: Create Debian package

create-rpm: seaos-PC.iso
	@echo "Creating Red Hat RPM package"
	# TODO: Create RPM package

create-msi: seaos-PC.iso
	@echo "Creating Windows MSI installer"
	# TODO: Create MSI installer

create-appx: seaos-PC.iso
	@echo "Creating Windows Store APPX package"
	# TODO: Create APPX package

# AI testing targets
test-ai-detection:
	@echo "Testing AI device detection"
	$(MAKE) debug-pc &
	$(MAKE) debug-tablet &
	$(MAKE) debug-mobile &

# Help
help:
	@echo "SeaOS Build System with AI"
	@echo "=========================="
	@echo ""
	@echo "Targets:"
	@echo "  all          - Build for default platform (PC)"
	@echo "  pc           - Build for PC platform"
	@echo "  pe           - Build for PE environment"
	@echo "  tablet       - Build for tablet platform"
	@echo "  mobile       - Build for mobile platform"
	@echo "  clean        - Clean build files"
	@echo ""
	@echo "Install targets:"
	@echo "  install-pc   - Install PC version"
	@echo "  install-pe   - Install PE version"
	@echo "  install-tablet - Install tablet version"
	@echo "  install-mobile - Install mobile version"
	@echo ""
	@echo "Package creation:"
	@echo "  create-exe   - Create Windows EXE installer"
	@echo "  create-apk   - Create Android APK package"
	@echo "  create-ipa   - Create iOS IPA package"
	@echo "  create-deb   - Create Debian DEB package"
	@echo "  create-rpm   - Create Red Hat RPM package"
	@echo "  create-msi   - Create Windows MSI installer"
	@echo "  create-appx  - Create Windows Store APPX package"
	@echo ""
	@echo "Debug targets:"
	@echo "  debug-pc     - Run PC version in QEMU"
	@echo "  debug-pe     - Run PE version in QEMU"
	@echo "  debug-tablet - Run tablet version in QEMU"
	@echo "  debug-mobile - Run mobile version in QEMU"
	@echo ""
	@echo "AI testing:"
	@echo "  test-ai-detection - Test AI device detection"
	@echo ""
	@echo "Variables:"
	@echo "  PLATFORM     - Target platform (PC, PE, TABLET, MOBILE)"
	@echo "  ARCH         - Target architecture (x86_64, aarch64)"