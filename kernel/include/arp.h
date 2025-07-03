#ifndef ARP_H
#define ARP_H

#include <stdint.h>

// ARP cache entry states
#define ARP_STATE_FREE     0
#define ARP_STATE_PENDING  1
#define ARP_STATE_RESOLVED 2

// ARP operations
#define ARP_REQUEST 1
#define ARP_REPLY   2

// ARP initialization
void arp_init(void);

// ARP operations
int arp_resolve(uint32_t ip_addr, uint8_t* mac_addr);
void arp_update(uint32_t ip_addr, const uint8_t* mac_addr);
void arp_handle_packet(const void* packet, size_t length);

// ARP cache management
void arp_cache_clear(void);
int arp_cache_lookup(uint32_t ip_addr, uint8_t* mac_addr);

#endif