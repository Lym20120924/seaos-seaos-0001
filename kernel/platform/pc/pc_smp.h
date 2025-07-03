#ifndef PC_SMP_H
#define PC_SMP_H

#include <stdint.h>

void smp_init(void);
void smp_start_aps(void);
void smp_send_ipi_all(uint32_t vector);
void smp_send_ipi(uint8_t cpu_id, uint32_t vector);
uint8_t smp_get_current_cpu_id(void);
int smp_get_cpu_count(void);
void smp_eoi(void);
void smp_ap_entry(void);

#endif