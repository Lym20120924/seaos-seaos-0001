#ifndef TABLET_SENSORS_H
#define TABLET_SENSORS_H

#include <stdint.h>

// Orientation constants
#define ORIENTATION_UNKNOWN           0
#define ORIENTATION_PORTRAIT          1
#define ORIENTATION_LANDSCAPE_LEFT    2
#define ORIENTATION_LANDSCAPE_RIGHT   3
#define ORIENTATION_PORTRAIT_UPSIDE_DOWN 4
#define ORIENTATION_FACE_UP           5
#define ORIENTATION_FACE_DOWN         6

void sensors_init(void);
int sensors_init_accelerometer(void);
int sensors_init_gyroscope(void);
int sensors_init_magnetometer(void);
int sensors_init_ambient_light(void);
int sensors_init_proximity(void);

void sensors_read_accelerometer(float* x, float* y, float* z);
void sensors_read_gyroscope(float* x, float* y, float* z);
void sensors_read_magnetometer(float* x, float* y, float* z);
float sensors_read_ambient_light(void);
float sensors_read_proximity(void);

int sensors_get_orientation(void);
int sensors_detect_motion(void);
int sensors_get_step_count(void);
void sensors_reset_step_count(void);

void sensors_calibrate_accelerometer(void);
void sensors_calibrate_magnetometer(void);

void sensors_enable(uint8_t sensor_mask);
void sensors_disable(uint8_t sensor_mask);
uint8_t sensors_get_enabled_mask(void);

#endif