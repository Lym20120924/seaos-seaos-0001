#ifndef UDP_H
#define UDP_H

#include <stdint.h>
#include <stddef.h>

// UDP statistics structure
typedef struct {
    uint32_t packets_sent;
    uint32_t packets_received;
    uint32_t packets_dropped;
    uint32_t checksum_errors;
    uint32_t listeners_count;
} udp_stats_t;

// UDP initialization
void udp_init(void);

// UDP listener management
int udp_register_listener(uint16_t port, 
    void (*handler)(const void* data, size_t length, uint32_t source_ip, uint16_t source_port));
void udp_unregister_listener(uint16_t port);

// UDP packet operations
int udp_send_packet(uint32_t dest_ip, uint16_t dest_port, uint16_t source_port, 
                   const void* data, size_t length);
void udp_handle_packet(const void* packet, size_t length, uint32_t source_ip);

// Utility functions
int udp_send_broadcast(uint16_t port, uint16_t source_port, const void* data, size_t length);
int udp_send_multicast(uint32_t multicast_ip, uint16_t port, uint16_t source_port, 
                      const void* data, size_t length);

// Statistics
void udp_get_stats(udp_stats_t* stats);

#endif