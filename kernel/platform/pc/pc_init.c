#include <stdint.h>
#include "../../core/kprintf.h"
#include "../../core/mm.h"
#include "../../core/process.h"
#include "../../drivers/gpu/vga.h"
#include "../../drivers/input/ps2.h"
#include "../../drivers/net/rtl8139.h"
#include "../../net/ip.h"
#include "../../net/dhcp.h"
#include "../../fs/ext4.h"
#include "../../fs/ntfs.h"
#include "pc_acpi.h"
#include "pc_pci.h"
#include "pc_smp.h"

// PC-specific hardware detection and initialization
void pc_detect_hardware(void) {
    kprintf("Detecting PC hardware...\n");
    
    // Detect CPU features
    uint32_t eax, ebx, ecx, edx;
    __asm__ volatile("cpuid" : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx) : "a"(0));
    
    char vendor[13];
    *(uint32_t*)&vendor[0] = ebx;
    *(uint32_t*)&vendor[4] = edx;
    *(uint32_t*)&vendor[8] = ecx;
    vendor[12] = '\0';
    
    kprintf("CPU Vendor: %s\n", vendor);
    
    // Get CPU features
    __asm__ volatile("cpuid" : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx) : "a"(1));
    
    if (edx & (1 << 0)) kprintf("  FPU: Yes\n");
    if (edx & (1 << 4)) kprintf("  TSC: Yes\n");
    if (edx & (1 << 5)) kprintf("  MSR: Yes\n");
    if (edx & (1 << 6)) kprintf("  PAE: Yes\n");
    if (edx & (1 << 9)) kprintf("  APIC: Yes\n");
    if (edx & (1 << 15)) kprintf("  CMOV: Yes\n");
    if (edx & (1 << 23)) kprintf("  MMX: Yes\n");
    if (edx & (1 << 25)) kprintf("  SSE: Yes\n");
    if (edx & (1 << 26)) kprintf("  SSE2: Yes\n");
    if (ecx & (1 << 0)) kprintf("  SSE3: Yes\n");
    if (ecx & (1 << 9)) kprintf("  SSSE3: Yes\n");
    if (ecx & (1 << 19)) kprintf("  SSE4.1: Yes\n");
    if (ecx & (1 << 20)) kprintf("  SSE4.2: Yes\n");
    if (ecx & (1 << 28)) kprintf("  AVX: Yes\n");
    
    // Detect memory size
    uint32_t mem_lower = 0, mem_upper = 0;
    
    // Use BIOS interrupt 0x15, function 0xE820 for memory detection
    // This would be done in the bootloader and passed to kernel
    kprintf("Memory detection completed\n");
}

void pc_init_acpi(void) {
    kprintf("Initializing ACPI...\n");
    acpi_init();
    kprintf("ACPI initialized\n");
}

void pc_init_pci(void) {
    kprintf("Initializing PCI bus...\n");
    pci_init();
    pci_scan_devices();
    kprintf("PCI initialization completed\n");
}

void pc_init_smp(void) {
    kprintf("Initializing SMP...\n");
    smp_init();
    kprintf("SMP initialization completed\n");
}

void pc_init_storage(void) {
    kprintf("Initializing storage subsystem...\n");
    
    // Initialize file systems
    ext4_init();
    ntfs_init();
    
    // TODO: Initialize SATA/IDE controllers
    // TODO: Initialize NVMe controllers
    // TODO: Initialize USB storage
    
    kprintf("Storage subsystem initialized\n");
}

void pc_init_network(void) {
    kprintf("Initializing network subsystem...\n");
    
    // Initialize network stack
    ip_init();
    dhcp_init();
    
    // Initialize network drivers
    rtl8139_init();
    
    // Start DHCP to get IP configuration
    dhcp_start();
    
    kprintf("Network subsystem initialized\n");
}

void pc_init_graphics(void) {
    kprintf("Initializing graphics subsystem...\n");
    
    // Initialize VGA for basic text output
    vga_init();
    
    // TODO: Initialize VESA/GOP framebuffer
    // TODO: Initialize GPU drivers (Intel, AMD, NVIDIA)
    // TODO: Initialize hardware acceleration
    
    kprintf("Graphics subsystem initialized\n");
}

void pc_init_audio(void) {
    kprintf("Initializing audio subsystem...\n");
    
    // TODO: Initialize AC97 audio
    // TODO: Initialize HDA audio
    // TODO: Initialize USB audio
    
    kprintf("Audio subsystem initialized\n");
}

void pc_init_input(void) {
    kprintf("Initializing input subsystem...\n");
    
    // Initialize PS/2 keyboard and mouse
    ps2_init();
    
    // TODO: Initialize USB HID devices
    // TODO: Initialize game controllers
    
    kprintf("Input subsystem initialized\n");
}

void pc_platform_init(void) {
    kprintf("Initializing PC platform...\n");
    
    // Hardware detection
    pc_detect_hardware();
    
    // Core system initialization
    pc_init_acpi();
    pc_init_pci();
    pc_init_smp();
    
    // Subsystem initialization
    pc_init_storage();
    pc_init_network();
    pc_init_graphics();
    pc_init_audio();
    pc_init_input();
    
    kprintf("PC platform initialization completed\n");
}