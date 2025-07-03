#ifndef PC_PCI_H
#define PC_PCI_H

#include <stdint.h>

typedef struct pci_device pci_device_t;

void pci_init(void);
void pci_scan_devices(void);
pci_device_t* pci_find_device(uint16_t vendor_id, uint16_t device_id);
pci_device_t* pci_find_device_by_class(uint8_t class_code, uint8_t subclass);
void pci_enable_device(pci_device_t* dev);
uint32_t pci_get_bar(pci_device_t* dev, int bar_index);
void pci_set_bar(pci_device_t* dev, int bar_index, uint32_t value);

#endif