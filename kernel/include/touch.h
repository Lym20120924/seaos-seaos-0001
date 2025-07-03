#ifndef TOUCH_H
#define TOUCH_H

#include <stdint.h>
#include <stddef.h>

typedef struct touch_point touch_point_t;

void touch_init(void);
int touch_process_event(uint8_t* event_data, size_t length);
void touch_get_point(uint8_t index, touch_point_t* point);
uint8_t touch_get_num_points(void);

#endif