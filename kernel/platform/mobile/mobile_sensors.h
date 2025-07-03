#ifndef MOBILE_SENSORS_H
#define MOBILE_SENSORS_H

#include <stdint.h>

void mobile_sensors_init(void);
int mobile_sensors_init_accelerometer(void);
int mobile_sensors_init_gyroscope(void);
int mobile_sensors_init_magnetometer(void);
int mobile_sensors_init_ambient_light(void);
int mobile_sensors_init_proximity(void);
int mobile_sensors_init_fingerprint(void);
int mobile_sensors_init_heart_rate(void);

void mobile_sensors_read_accelerometer(float* x, float* y, float* z);
void mobile_sensors_read_gyroscope(float* x, float* y, float* z);
void mobile_sensors_read_magnetometer(float* x, float* y, float* z);
float mobile_sensors_read_ambient_light(void);
float mobile_sensors_read_proximity(void);
int mobile_sensors_read_fingerprint(void);
int mobile_sensors_read_heart_rate(void);

#endif