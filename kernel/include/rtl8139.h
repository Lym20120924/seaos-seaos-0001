#ifndef RTL8139_H
#define RTL8139_H

#include <stddef.h>

void rtl8139_init(void);
int rtl8139_send_packet(const void* data, size_t length);
int rtl8139_receive_packet(void* buffer, size_t max_length);

#endif