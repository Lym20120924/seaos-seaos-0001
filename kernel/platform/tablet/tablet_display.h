#ifndef TABLET_DISPLAY_H
#define TABLET_DISPLAY_H

#include <stdint.h>

#define DISPLAY_ORIENTATION_PORTRAIT          0
#define DISPLAY_ORIENTATION_LANDSCAPE_LEFT    1
#define DISPLAY_ORIENTATION_LANDSCAPE_RIGHT   2
#define DISPLAY_ORIENTATION_PORTRAIT_UPSIDE_DOWN 3

void display_init(void);
void display_set_orientation(int orientation);
int display_get_orientation(void);
void display_set_brightness(int brightness);
int display_get_brightness(void);
void display_enable_auto_brightness(int enable);
void display_set_resolution(int width, int height);
void display_get_resolution(int* width, int* height);

#endif