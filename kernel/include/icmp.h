#ifndef ICMP_H
#define ICMP_H

#include <stdint.h>
#include <stddef.h>

// ICMP message types
#define ICMP_ECHO_REPLY      0
#define ICMP_DEST_UNREACHABLE 3
#define ICMP_ECHO_REQUEST    8
#define ICMP_TIME_EXCEEDED   11

// ICMP initialization
void icmp_init(void);

// ICMP packet operations
int icmp_send_echo_request(uint32_t dest_ip, uint16_t sequence, uint16_t identifier);
void icmp_handle_packet(const void* packet, size_t length, uint32_t source_ip);

// ICMP callback registration
typedef void (*icmp_callback_t)(uint8_t type, uint8_t code, const void* data, size_t length, uint32_t source_ip);
void icmp_register_callback(icmp_callback_t callback);

#endif