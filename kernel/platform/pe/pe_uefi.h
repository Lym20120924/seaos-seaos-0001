#ifndef PE_UEFI_H
#define PE_UEFI_H

#include <stdint.h>

void uefi_init(void);
int uefi_get_memory_map(void);
int uefi_exit_boot_services(void);
void* uefi_allocate_pages(uint64_t pages);
void uefi_free_pages(void* address, uint64_t pages);

#endif