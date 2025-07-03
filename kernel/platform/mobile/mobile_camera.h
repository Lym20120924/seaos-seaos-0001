#ifndef MOBILE_CAMERA_H
#define MOBILE_CAMERA_H

#include <stdint.h>

void mobile_camera_init(void);
int mobile_camera_init_front(void);
int mobile_camera_init_rear(void);

void mobile_camera_enable_auto_focus(int enable);
void mobile_camera_enable_image_stabilization(int enable);
void mobile_camera_enable_hdr(int enable);

#endif