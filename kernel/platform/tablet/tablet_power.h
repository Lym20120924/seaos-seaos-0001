#ifndef TABLET_POWER_H
#define TABLET_POWER_H

#include <stdint.h>

#define TABLET_POWER_PROFILE_PERFORMANCE  0
#define TABLET_POWER_PROFILE_BALANCED     1
#define TABLET_POWER_PROFILE_POWER_SAVER  2

void tablet_power_init(void);
void tablet_power_set_profile(int profile);
int tablet_power_get_profile(void);

void tablet_power_enable_dvfs(int enable);
void tablet_power_configure_sleep(void);

int tablet_power_init_battery(void);
int tablet_power_get_battery_level(void);
int tablet_power_is_charging(void);
int tablet_power_get_time_remaining(void);

void tablet_power_enter_sleep(void);
void tablet_power_wake_from_sleep(void);
void tablet_power_deep_sleep(void);

#endif