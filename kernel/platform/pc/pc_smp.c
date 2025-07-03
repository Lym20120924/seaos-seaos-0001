#include <stdint.h>
#include "../../core/kprintf.h"
#include "../../core/mm.h"
#include "pc_smp.h"
#include "pc_acpi.h"

#define MAX_CPUS 256
#define APIC_BASE_MSR 0x1B
#define APIC_ENABLE_BIT (1 << 11)

// Local APIC registers
#define APIC_ID_REG           0x020
#define APIC_VERSION_REG      0x030
#define APIC_TPR_REG          0x080
#define APIC_APR_REG          0x090
#define APIC_PPR_REG          0x0A0
#define APIC_EOI_REG          0x0B0
#define APIC_RRD_REG          0x0C0
#define APIC_LDR_REG          0x0D0
#define APIC_DFR_REG          0x0E0
#define APIC_SPURIOUS_REG     0x0F0
#define APIC_ESR_REG          0x280
#define APIC_ICR_LOW_REG      0x300
#define APIC_ICR_HIGH_REG     0x310
#define APIC_LVT_TIMER_REG    0x320
#define APIC_LVT_THERMAL_REG  0x330
#define APIC_LVT_PERF_REG     0x340
#define APIC_LVT_LINT0_REG    0x350
#define APIC_LVT_LINT1_REG    0x360
#define APIC_LVT_ERROR_REG    0x370
#define APIC_TIMER_ICR_REG    0x380
#define APIC_TIMER_CCR_REG    0x390
#define APIC_TIMER_DCR_REG    0x3E0

// ICR delivery modes
#define ICR_DELIVERY_FIXED    0x0
#define ICR_DELIVERY_INIT     0x5
#define ICR_DELIVERY_STARTUP  0x6

// ICR destination modes
#define ICR_DEST_PHYSICAL     0x0
#define ICR_DEST_LOGICAL      0x1

// ICR level
#define ICR_LEVEL_DEASSERT    0x0
#define ICR_LEVEL_ASSERT      0x1

// ICR trigger mode
#define ICR_TRIGGER_EDGE      0x0
#define ICR_TRIGGER_LEVEL     0x1

typedef struct {
    uint8_t apic_id;
    uint8_t processor_id;
    uint8_t enabled;
    uint8_t bsp;  // Bootstrap processor
} cpu_info_t;

static uint32_t* local_apic_base = NULL;
static cpu_info_t cpus[MAX_CPUS];
static int cpu_count = 0;
static int bsp_id = 0;

static uint32_t apic_read(uint32_t reg) {
    return *(volatile uint32_t*)((uint8_t*)local_apic_base + reg);
}

static void apic_write(uint32_t reg, uint32_t value) {
    *(volatile uint32_t*)((uint8_t*)local_apic_base + reg) = value;
}

static void apic_send_ipi(uint8_t dest_apic_id, uint32_t vector, uint32_t delivery_mode) {
    // Wait for any pending IPI to complete
    while (apic_read(APIC_ICR_LOW_REG) & (1 << 12));
    
    // Set destination
    apic_write(APIC_ICR_HIGH_REG, (uint32_t)dest_apic_id << 24);
    
    // Send IPI
    uint32_t icr_low = vector | (delivery_mode << 8) | (ICR_DEST_PHYSICAL << 11) | 
                       (ICR_LEVEL_ASSERT << 14) | (ICR_TRIGGER_EDGE << 15);
    apic_write(APIC_ICR_LOW_REG, icr_low);
    
    // Wait for IPI to be sent
    while (apic_read(APIC_ICR_LOW_REG) & (1 << 12));
}

static void parse_madt_entries(void) {
    // This would parse MADT entries from ACPI
    // For now, we'll simulate a single CPU system
    cpus[0].apic_id = 0;
    cpus[0].processor_id = 0;
    cpus[0].enabled = 1;
    cpus[0].bsp = 1;
    cpu_count = 1;
    bsp_id = 0;
    
    kprintf("Found %d CPU(s)\n", cpu_count);
    for (int i = 0; i < cpu_count; i++) {
        kprintf("  CPU %d: APIC ID %d %s\n", 
                cpus[i].processor_id, cpus[i].apic_id,
                cpus[i].bsp ? "(BSP)" : "");
    }
}

void smp_init(void) {
    kprintf("Initializing SMP...\n");
    
    // Get Local APIC base address from MSR
    uint64_t apic_base_msr;
    __asm__ volatile("rdmsr" : "=A"(apic_base_msr) : "c"(APIC_BASE_MSR));
    
    local_apic_base = (uint32_t*)(apic_base_msr & 0xFFFFF000);
    kprintf("Local APIC base: %p\n", local_apic_base);
    
    // Enable Local APIC
    if (!(apic_base_msr & APIC_ENABLE_BIT)) {
        apic_base_msr |= APIC_ENABLE_BIT;
        __asm__ volatile("wrmsr" : : "A"(apic_base_msr), "c"(APIC_BASE_MSR));
    }
    
    // Initialize Local APIC
    apic_write(APIC_SPURIOUS_REG, 0x100 | 0xFF);  // Enable APIC + spurious vector
    apic_write(APIC_TPR_REG, 0);  // Accept all interrupts
    
    // Parse CPU information from ACPI MADT
    parse_madt_entries();
    
    // Start Application Processors (APs)
    if (cpu_count > 1) {
        smp_start_aps();
    }
    
    kprintf("SMP initialization completed\n");
}

void smp_start_aps(void) {
    kprintf("Starting Application Processors...\n");
    
    for (int i = 0; i < cpu_count; i++) {
        if (cpus[i].bsp || !cpus[i].enabled) continue;
        
        kprintf("Starting CPU %d (APIC ID %d)...\n", 
                cpus[i].processor_id, cpus[i].apic_id);
        
        // Send INIT IPI
        apic_send_ipi(cpus[i].apic_id, 0, ICR_DELIVERY_INIT);
        
        // Wait 10ms
        // TODO: Implement proper delay
        
        // Send STARTUP IPI
        uint32_t startup_vector = 0x08;  // Start at 0x8000
        apic_send_ipi(cpus[i].apic_id, startup_vector, ICR_DELIVERY_STARTUP);
        
        // Wait 200μs
        // TODO: Implement proper delay
        
        // Send second STARTUP IPI
        apic_send_ipi(cpus[i].apic_id, startup_vector, ICR_DELIVERY_STARTUP);
        
        // TODO: Wait for AP to signal it's ready
    }
}

void smp_send_ipi_all(uint32_t vector) {
    for (int i = 0; i < cpu_count; i++) {
        if (cpus[i].apic_id == smp_get_current_cpu_id()) continue;
        if (!cpus[i].enabled) continue;
        
        apic_send_ipi(cpus[i].apic_id, vector, ICR_DELIVERY_FIXED);
    }
}

void smp_send_ipi(uint8_t cpu_id, uint32_t vector) {
    if (cpu_id >= cpu_count || !cpus[cpu_id].enabled) return;
    
    apic_send_ipi(cpus[cpu_id].apic_id, vector, ICR_DELIVERY_FIXED);
}

uint8_t smp_get_current_cpu_id(void) {
    uint32_t apic_id = (apic_read(APIC_ID_REG) >> 24) & 0xFF;
    
    for (int i = 0; i < cpu_count; i++) {
        if (cpus[i].apic_id == apic_id) {
            return cpus[i].processor_id;
        }
    }
    
    return 0;  // Default to BSP
}

int smp_get_cpu_count(void) {
    return cpu_count;
}

void smp_eoi(void) {
    apic_write(APIC_EOI_REG, 0);
}

void smp_ap_entry(void) {
    // This function is called by Application Processors when they start
    kprintf("AP %d started\n", smp_get_current_cpu_id());
    
    // Initialize Local APIC for this CPU
    apic_write(APIC_SPURIOUS_REG, 0x100 | 0xFF);
    apic_write(APIC_TPR_REG, 0);
    
    // TODO: Initialize per-CPU data structures
    // TODO: Enable interrupts
    // TODO: Enter scheduler
    
    while (1) {
        __asm__ volatile("hlt");
    }
}