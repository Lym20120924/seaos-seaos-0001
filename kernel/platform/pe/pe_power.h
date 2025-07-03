#ifndef PE_POWER_H
#define PE_POWER_H

#include <stdint.h>

#define POWER_PROFILE_PERFORMANCE 0
#define POWER_PROFILE_BALANCED    1
#define POWER_PROFILE_POWER_SAVER 2

void power_init(void);
void power_set_profile(int profile);
int power_get_profile(void);
void power_shutdown(void);
void power_reboot(void);

#endif