#ifndef MOBILE_DISPLAY_H
#define MOBILE_DISPLAY_H

#include <stdint.h>

#define MOBILE_DISPLAY_PORTRAIT          0
#define MOBILE_DISPLAY_LANDSCAPE_LEFT    1
#define MOBILE_DISPLAY_LANDSCAPE_RIGHT   2
#define MOBILE_DISPLAY_PORTRAIT_UPSIDE_DOWN 3

void mobile_display_init(void);
void mobile_display_set_orientation(int orientation);
int mobile_display_get_orientation(void);
void mobile_display_set_brightness(int brightness);
int mobile_display_get_brightness(void);
void mobile_display_enable_auto_brightness(int enable);
void mobile_display_enable_always_on_display(int enable);

#endif