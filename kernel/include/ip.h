#ifndef IP_H
#define IP_H

#include <stdint.h>
#include <stddef.h>

// IP protocol numbers
#define IP_PROTO_ICMP  1
#define IP_PROTO_TCP   6
#define IP_PROTO_UDP   17

// IP initialization
void ip_init(void);

// IP packet operations
int ip_send_packet(uint32_t dest_ip, uint8_t protocol, const void* data, size_t length);
void ip_handle_packet(const void* packet, size_t length);

// IP configuration
void ip_set_address(uint32_t ip, uint32_t mask, uint32_t gateway);
uint32_t ip_get_local_address(void);
uint32_t ip_get_subnet_mask(void);
uint32_t ip_get_gateway_address(void);

// Utility functions
const char* ip_to_string(uint32_t ip);
uint32_t string_to_ip(const char* str);

// Statistics
typedef struct {
    uint32_t packets_sent;
    uint32_t packets_received;
    uint32_t packets_forwarded;
    uint32_t packets_dropped;
    uint32_t checksum_errors;
    uint32_t fragmentation_errors;
} ip_stats_t;

void ip_get_stats(ip_stats_t* stats);

#endif