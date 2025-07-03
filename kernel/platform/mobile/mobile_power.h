#ifndef MOBILE_POWER_H
#define MOBILE_POWER_H

#include <stdint.h>

#define MOBILE_POWER_PROFILE_PERFORMANCE  0
#define MOBILE_POWER_PROFILE_BALANCED     1
#define MOBILE_POWER_PROFILE_POWER_SAVER  2
#define MOBILE_POWER_PROFILE_ADAPTIVE     3

void mobile_power_init(void);
void mobile_power_set_profile(int profile);
int mobile_power_get_profile(void);

void mobile_power_enable_doze_mode(int enable);
void mobile_power_enable_app_standby(int enable);
void mobile_power_enable_background_app_refresh(int enable);

int mobile_power_init_battery(void);
int mobile_power_get_battery_level(void);
int mobile_power_is_charging(void);
int mobile_power_is_fast_charging(void);
int mobile_power_is_wireless_charging(void);
int mobile_power_get_battery_health(void);

#endif