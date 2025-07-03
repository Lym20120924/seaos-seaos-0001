#ifndef PC_ACPI_H
#define PC_ACPI_H

#include <stdint.h>

void acpi_init(void);
uint32_t acpi_get_local_apic_address(void);
int acpi_enable(void);
void acpi_shutdown(void);
void acpi_reboot(void);

#endif