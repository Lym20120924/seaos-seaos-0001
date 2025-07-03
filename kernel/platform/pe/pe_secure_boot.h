#ifndef PE_SECURE_BOOT_H
#define PE_SECURE_BOOT_H

#include <stdint.h>

void secure_boot_init(void);
int secure_boot_is_enabled(void);
int secure_boot_verify_kernel(void);
int secure_boot_verify_module(const void* module, size_t size);

#endif