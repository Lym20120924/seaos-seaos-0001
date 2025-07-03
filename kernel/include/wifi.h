#ifndef WIFI_H
#define WIFI_H

#include <stdint.h>

typedef struct wifi_state wifi_state_t;

void wifi_init(void);
int wifi_scan_networks(void);
int wifi_connect(const char* ssid, const char* password);
void wifi_disconnect(void);
int wifi_get_status(wifi_state_t* status);

#endif