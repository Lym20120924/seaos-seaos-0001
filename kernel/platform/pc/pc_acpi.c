#include <stdint.h>
#include "../../core/kprintf.h"
#include "../../core/mm.h"
#include "pc_acpi.h"

#define ACPI_RSDP_SIGNATURE "RSD PTR "
#define ACPI_RSDT_SIGNATURE "RSDT"
#define ACPI_XSDT_SIGNATURE "XSDT"
#define ACPI_FADT_SIGNATURE "FACP"
#define ACPI_MADT_SIGNATURE "APIC"
#define ACPI_HPET_SIGNATURE "HPET"

typedef struct {
    char signature[8];
    uint8_t checksum;
    char oem_id[6];
    uint8_t revision;
    uint32_t rsdt_address;
    uint32_t length;
    uint64_t xsdt_address;
    uint8_t extended_checksum;
    uint8_t reserved[3];
} __attribute__((packed)) acpi_rsdp_t;

typedef struct {
    char signature[4];
    uint32_t length;
    uint8_t revision;
    uint8_t checksum;
    char oem_id[6];
    char oem_table_id[8];
    uint32_t oem_revision;
    uint32_t creator_id;
    uint32_t creator_revision;
} __attribute__((packed)) acpi_sdt_header_t;

typedef struct {
    acpi_sdt_header_t header;
    uint32_t entries[];
} __attribute__((packed)) acpi_rsdt_t;

typedef struct {
    acpi_sdt_header_t header;
    uint64_t entries[];
} __attribute__((packed)) acpi_xsdt_t;

typedef struct {
    acpi_sdt_header_t header;
    uint32_t firmware_ctrl;
    uint32_t dsdt;
    uint8_t reserved;
    uint8_t preferred_pm_profile;
    uint16_t sci_interrupt;
    uint32_t smi_command_port;
    uint8_t acpi_enable;
    uint8_t acpi_disable;
    uint8_t s4bios_req;
    uint8_t pstate_control;
    uint32_t pm1a_event_block;
    uint32_t pm1b_event_block;
    uint32_t pm1a_control_block;
    uint32_t pm1b_control_block;
    uint32_t pm2_control_block;
    uint32_t pm_timer_block;
    uint32_t gpe0_block;
    uint32_t gpe1_block;
    uint8_t pm1_event_length;
    uint8_t pm1_control_length;
    uint8_t pm2_control_length;
    uint8_t pm_timer_length;
    uint8_t gpe0_length;
    uint8_t gpe1_length;
    uint8_t gpe1_base;
    uint8_t cstate_control;
    uint16_t worst_c2_latency;
    uint16_t worst_c3_latency;
    uint16_t flush_size;
    uint16_t flush_stride;
    uint8_t duty_offset;
    uint8_t duty_width;
    uint8_t day_alarm;
    uint8_t month_alarm;
    uint8_t century;
    uint16_t boot_architecture_flags;
    uint8_t reserved2;
    uint32_t flags;
} __attribute__((packed)) acpi_fadt_t;

typedef struct {
    acpi_sdt_header_t header;
    uint32_t local_apic_address;
    uint32_t flags;
    uint8_t entries[];
} __attribute__((packed)) acpi_madt_t;

static acpi_rsdp_t* rsdp = NULL;
static acpi_rsdt_t* rsdt = NULL;
static acpi_xsdt_t* xsdt = NULL;
static acpi_fadt_t* fadt = NULL;
static acpi_madt_t* madt = NULL;

static uint8_t acpi_checksum(void* table, size_t length) {
    uint8_t sum = 0;
    uint8_t* bytes = (uint8_t*)table;
    for (size_t i = 0; i < length; i++) {
        sum += bytes[i];
    }
    return sum;
}

static acpi_rsdp_t* find_rsdp(void) {
    // Search in EBDA (Extended BIOS Data Area)
    uint16_t ebda = *(uint16_t*)0x40E;
    if (ebda) {
        uint8_t* ebda_ptr = (uint8_t*)(ebda << 4);
        for (int i = 0; i < 1024; i += 16) {
            if (memcmp(ebda_ptr + i, ACPI_RSDP_SIGNATURE, 8) == 0) {
                acpi_rsdp_t* candidate = (acpi_rsdp_t*)(ebda_ptr + i);
                if (acpi_checksum(candidate, 20) == 0) {
                    return candidate;
                }
            }
        }
    }
    
    // Search in BIOS ROM area (0xE0000 - 0xFFFFF)
    for (uint32_t addr = 0xE0000; addr < 0x100000; addr += 16) {
        if (memcmp((void*)addr, ACPI_RSDP_SIGNATURE, 8) == 0) {
            acpi_rsdp_t* candidate = (acpi_rsdp_t*)addr;
            if (acpi_checksum(candidate, 20) == 0) {
                return candidate;
            }
        }
    }
    
    return NULL;
}

static void* find_acpi_table(const char* signature) {
    if (!rsdt && !xsdt) return NULL;
    
    if (xsdt) {
        uint32_t entries = (xsdt->header.length - sizeof(acpi_sdt_header_t)) / 8;
        for (uint32_t i = 0; i < entries; i++) {
            acpi_sdt_header_t* header = (acpi_sdt_header_t*)xsdt->entries[i];
            if (memcmp(header->signature, signature, 4) == 0) {
                if (acpi_checksum(header, header->length) == 0) {
                    return header;
                }
            }
        }
    } else if (rsdt) {
        uint32_t entries = (rsdt->header.length - sizeof(acpi_sdt_header_t)) / 4;
        for (uint32_t i = 0; i < entries; i++) {
            acpi_sdt_header_t* header = (acpi_sdt_header_t*)rsdt->entries[i];
            if (memcmp(header->signature, signature, 4) == 0) {
                if (acpi_checksum(header, header->length) == 0) {
                    return header;
                }
            }
        }
    }
    
    return NULL;
}

void acpi_init(void) {
    kprintf("Searching for ACPI RSDP...\n");
    
    rsdp = find_rsdp();
    if (!rsdp) {
        kprintf("ACPI RSDP not found\n");
        return;
    }
    
    kprintf("ACPI RSDP found at %p\n", rsdp);
    kprintf("ACPI Revision: %d\n", rsdp->revision);
    
    // Use XSDT if available (ACPI 2.0+), otherwise use RSDT
    if (rsdp->revision >= 2 && rsdp->xsdt_address) {
        xsdt = (acpi_xsdt_t*)rsdp->xsdt_address;
        kprintf("Using XSDT at %p\n", xsdt);
    } else {
        rsdt = (acpi_rsdt_t*)rsdp->rsdt_address;
        kprintf("Using RSDT at %p\n", rsdt);
    }
    
    // Find important ACPI tables
    fadt = (acpi_fadt_t*)find_acpi_table(ACPI_FADT_SIGNATURE);
    if (fadt) {
        kprintf("FADT found at %p\n", fadt);
        kprintf("PM Timer Block: 0x%x\n", fadt->pm_timer_block);
        kprintf("SCI Interrupt: %d\n", fadt->sci_interrupt);
    }
    
    madt = (acpi_madt_t*)find_acpi_table(ACPI_MADT_SIGNATURE);
    if (madt) {
        kprintf("MADT found at %p\n", madt);
        kprintf("Local APIC Address: 0x%x\n", madt->local_apic_address);
    }
    
    void* hpet = find_acpi_table(ACPI_HPET_SIGNATURE);
    if (hpet) {
        kprintf("HPET found at %p\n", hpet);
    }
}

uint32_t acpi_get_local_apic_address(void) {
    return madt ? madt->local_apic_address : 0;
}

int acpi_enable(void) {
    if (!fadt) return -1;
    
    if (fadt->smi_command_port && fadt->acpi_enable) {
        outb(fadt->smi_command_port, fadt->acpi_enable);
        
        // Wait for ACPI to be enabled
        while (!(inw(fadt->pm1a_control_block) & 1)) {
            // Wait
        }
        
        kprintf("ACPI enabled\n");
        return 0;
    }
    
    return -1;
}

void acpi_shutdown(void) {
    if (!fadt) return;
    
    // Try ACPI shutdown
    if (fadt->pm1a_control_block) {
        outw(fadt->pm1a_control_block, 0x2000);
    }
    
    // Fallback to older methods
    outw(0x604, 0x2000);  // QEMU
    outw(0xB004, 0x2000); // Bochs
    outw(0x4004, 0x3400); // VirtualBox
}

void acpi_reboot(void) {
    // Try ACPI reset
    if (fadt && fadt->flags & (1 << 10)) {
        // ACPI reset supported
        outb(0x64, 0xFE);
    }
    
    // Fallback methods
    outb(0x64, 0xFE);     // Keyboard controller
    outb(0x92, 0x01);     // Fast A20 gate
    
    // Triple fault as last resort
    __asm__ volatile("cli; hlt");
}