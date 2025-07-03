#ifndef MOBILE_GPS_H
#define MOBILE_GPS_H

#include <stdint.h>

typedef struct {
    double latitude;
    double longitude;
    double altitude;
    float accuracy;
    uint64_t timestamp;
} gps_location_t;

int mobile_gps_init(void);
int mobile_gps_get_location(gps_location_t* location);
void mobile_gps_enable(int enable);
int mobile_gps_is_enabled(void);

#endif