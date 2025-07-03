#include <stdint.h>
#include "../../core/kprintf.h"
#include "pc_pci.h"

#define PCI_CONFIG_ADDRESS 0xCF8
#define PCI_CONFIG_DATA    0xCFC

#define PCI_VENDOR_ID      0x00
#define PCI_DEVICE_ID      0x02
#define PCI_COMMAND        0x04
#define PCI_STATUS         0x06
#define PCI_CLASS_CODE     0x0B
#define PCI_SUBCLASS       0x0A
#define PCI_PROG_IF        0x09
#define PCI_REVISION_ID    0x08
#define PCI_HEADER_TYPE    0x0E
#define PCI_BAR0           0x10
#define PCI_BAR1           0x14
#define PCI_BAR2           0x18
#define PCI_BAR3           0x1C
#define PCI_BAR4           0x20
#define PCI_BAR5           0x24

typedef struct pci_device {
    uint8_t bus;
    uint8_t device;
    uint8_t function;
    uint16_t vendor_id;
    uint16_t device_id;
    uint8_t class_code;
    uint8_t subclass;
    uint8_t prog_if;
    uint32_t bars[6];
    struct pci_device* next;
} pci_device_t;

static pci_device_t* pci_devices = NULL;

static uint32_t pci_config_read_dword(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset) {
    uint32_t address = (1 << 31) | (bus << 16) | (device << 11) | (function << 8) | (offset & 0xFC);
    outl(PCI_CONFIG_ADDRESS, address);
    return inl(PCI_CONFIG_DATA);
}

static uint16_t pci_config_read_word(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset) {
    uint32_t dword = pci_config_read_dword(bus, device, function, offset);
    return (dword >> ((offset & 2) * 8)) & 0xFFFF;
}

static uint8_t pci_config_read_byte(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset) {
    uint32_t dword = pci_config_read_dword(bus, device, function, offset);
    return (dword >> ((offset & 3) * 8)) & 0xFF;
}

static void pci_config_write_dword(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset, uint32_t value) {
    uint32_t address = (1 << 31) | (bus << 16) | (device << 11) | (function << 8) | (offset & 0xFC);
    outl(PCI_CONFIG_ADDRESS, address);
    outl(PCI_CONFIG_DATA, value);
}

static void pci_config_write_word(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset, uint16_t value) {
    uint32_t dword = pci_config_read_dword(bus, device, function, offset);
    uint32_t shift = (offset & 2) * 8;
    dword = (dword & ~(0xFFFF << shift)) | (value << shift);
    pci_config_write_dword(bus, device, function, offset, dword);
}

static void pci_config_write_byte(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset, uint8_t value) {
    uint32_t dword = pci_config_read_dword(bus, device, function, offset);
    uint32_t shift = (offset & 3) * 8;
    dword = (dword & ~(0xFF << shift)) | (value << shift);
    pci_config_write_dword(bus, device, function, offset, dword);
}

static const char* pci_get_class_name(uint8_t class_code, uint8_t subclass) {
    switch (class_code) {
        case 0x00: return "Unclassified";
        case 0x01:
            switch (subclass) {
                case 0x00: return "SCSI Controller";
                case 0x01: return "IDE Controller";
                case 0x02: return "Floppy Controller";
                case 0x03: return "IPI Controller";
                case 0x04: return "RAID Controller";
                case 0x05: return "ATA Controller";
                case 0x06: return "SATA Controller";
                case 0x07: return "SAS Controller";
                case 0x08: return "NVMe Controller";
                default: return "Mass Storage Controller";
            }
        case 0x02:
            switch (subclass) {
                case 0x00: return "Ethernet Controller";
                case 0x01: return "Token Ring Controller";
                case 0x02: return "FDDI Controller";
                case 0x03: return "ATM Controller";
                case 0x04: return "ISDN Controller";
                case 0x05: return "WorldFip Controller";
                case 0x06: return "PICMG Controller";
                case 0x80: return "Other Network Controller";
                default: return "Network Controller";
            }
        case 0x03:
            switch (subclass) {
                case 0x00: return "VGA Controller";
                case 0x01: return "XGA Controller";
                case 0x02: return "3D Controller";
                case 0x80: return "Other Display Controller";
                default: return "Display Controller";
            }
        case 0x04: return "Multimedia Controller";
        case 0x05: return "Memory Controller";
        case 0x06:
            switch (subclass) {
                case 0x00: return "Host Bridge";
                case 0x01: return "ISA Bridge";
                case 0x02: return "EISA Bridge";
                case 0x03: return "MCA Bridge";
                case 0x04: return "PCI-to-PCI Bridge";
                case 0x05: return "PCMCIA Bridge";
                case 0x06: return "NuBus Bridge";
                case 0x07: return "CardBus Bridge";
                case 0x08: return "RACEway Bridge";
                case 0x09: return "Semi-transparent PCI-to-PCI Bridge";
                case 0x0A: return "InfiniBand-to-PCI Bridge";
                case 0x80: return "Other Bridge";
                default: return "Bridge";
            }
        case 0x07: return "Communication Controller";
        case 0x08: return "System Peripheral";
        case 0x09: return "Input Device Controller";
        case 0x0A: return "Docking Station";
        case 0x0B: return "Processor";
        case 0x0C:
            switch (subclass) {
                case 0x00: return "FireWire Controller";
                case 0x01: return "ACCESS Bus Controller";
                case 0x02: return "SSA Controller";
                case 0x03: return "USB Controller";
                case 0x04: return "Fibre Channel Controller";
                case 0x05: return "SMBus Controller";
                case 0x06: return "InfiniBand Controller";
                case 0x07: return "IPMI Controller";
                case 0x08: return "SERCOS Controller";
                case 0x09: return "CANbus Controller";
                case 0x80: return "Other Serial Bus Controller";
                default: return "Serial Bus Controller";
            }
        case 0x0D: return "Wireless Controller";
        case 0x0E: return "Intelligent Controller";
        case 0x0F: return "Satellite Controller";
        case 0x10: return "Encryption Controller";
        case 0x11: return "Signal Processing Controller";
        case 0x12: return "Processing Accelerator";
        case 0x13: return "Non-Essential Instrumentation";
        case 0xFF: return "Unassigned Class";
        default: return "Unknown";
    }
}

static void pci_add_device(uint8_t bus, uint8_t device, uint8_t function) {
    uint16_t vendor_id = pci_config_read_word(bus, device, function, PCI_VENDOR_ID);
    if (vendor_id == 0xFFFF) return;  // No device
    
    pci_device_t* dev = (pci_device_t*)kmalloc(sizeof(pci_device_t));
    if (!dev) return;
    
    dev->bus = bus;
    dev->device = device;
    dev->function = function;
    dev->vendor_id = vendor_id;
    dev->device_id = pci_config_read_word(bus, device, function, PCI_DEVICE_ID);
    dev->class_code = pci_config_read_byte(bus, device, function, PCI_CLASS_CODE);
    dev->subclass = pci_config_read_byte(bus, device, function, PCI_SUBCLASS);
    dev->prog_if = pci_config_read_byte(bus, device, function, PCI_PROG_IF);
    
    // Read BARs
    for (int i = 0; i < 6; i++) {
        dev->bars[i] = pci_config_read_dword(bus, device, function, PCI_BAR0 + i * 4);
    }
    
    // Add to device list
    dev->next = pci_devices;
    pci_devices = dev;
    
    kprintf("PCI %02x:%02x.%x %04x:%04x %s\n",
            bus, device, function, vendor_id, dev->device_id,
            pci_get_class_name(dev->class_code, dev->subclass));
}

void pci_init(void) {
    kprintf("Initializing PCI subsystem...\n");
    pci_devices = NULL;
}

void pci_scan_devices(void) {
    kprintf("Scanning PCI devices...\n");
    
    for (uint16_t bus = 0; bus < 256; bus++) {
        for (uint8_t device = 0; device < 32; device++) {
            uint16_t vendor_id = pci_config_read_word(bus, device, 0, PCI_VENDOR_ID);
            if (vendor_id == 0xFFFF) continue;
            
            uint8_t header_type = pci_config_read_byte(bus, device, 0, PCI_HEADER_TYPE);
            
            // Function 0 always exists if device exists
            pci_add_device(bus, device, 0);
            
            // Check for multi-function device
            if (header_type & 0x80) {
                for (uint8_t function = 1; function < 8; function++) {
                    uint16_t func_vendor = pci_config_read_word(bus, device, function, PCI_VENDOR_ID);
                    if (func_vendor != 0xFFFF) {
                        pci_add_device(bus, device, function);
                    }
                }
            }
        }
    }
}

pci_device_t* pci_find_device(uint16_t vendor_id, uint16_t device_id) {
    pci_device_t* dev = pci_devices;
    while (dev) {
        if (dev->vendor_id == vendor_id && dev->device_id == device_id) {
            return dev;
        }
        dev = dev->next;
    }
    return NULL;
}

pci_device_t* pci_find_device_by_class(uint8_t class_code, uint8_t subclass) {
    pci_device_t* dev = pci_devices;
    while (dev) {
        if (dev->class_code == class_code && dev->subclass == subclass) {
            return dev;
        }
        dev = dev->next;
    }
    return NULL;
}

void pci_enable_device(pci_device_t* dev) {
    if (!dev) return;
    
    uint16_t command = pci_config_read_word(dev->bus, dev->device, dev->function, PCI_COMMAND);
    command |= 0x07;  // Enable I/O space, memory space, and bus mastering
    pci_config_write_word(dev->bus, dev->device, dev->function, PCI_COMMAND, command);
}

uint32_t pci_get_bar(pci_device_t* dev, int bar_index) {
    if (!dev || bar_index < 0 || bar_index >= 6) return 0;
    return dev->bars[bar_index];
}

void pci_set_bar(pci_device_t* dev, int bar_index, uint32_t value) {
    if (!dev || bar_index < 0 || bar_index >= 6) return;
    
    pci_config_write_dword(dev->bus, dev->device, dev->function, 
                          PCI_BAR0 + bar_index * 4, value);
    dev->bars[bar_index] = value;
}