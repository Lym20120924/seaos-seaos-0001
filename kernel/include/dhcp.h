#ifndef DHCP_H
#define DHCP_H

#include <stdint.h>

// DHCP message types
#define DHCP_DISCOVER 1
#define DHCP_OFFER    2
#define DHCP_REQUEST  3
#define DHCP_ACK      4
#define DHCP_NAK      5
#define DHCP_RELEASE  6

// DHCP configuration
typedef struct {
    uint32_t ip_address;
    uint32_t subnet_mask;
    uint32_t gateway;
    uint32_t dns_server;
    uint32_t lease_time;
} dhcp_config_t;

// DHCP client functions
void dhcp_init(void);
int dhcp_start(void);
void dhcp_release(void);
void dhcp_renew(void);
int dhcp_is_bound(void);
int dhcp_get_state(void);
const dhcp_config_t* dhcp_get_config(void);

// DHCP packet handling
void dhcp_handle_packet(const void* packet, size_t length, uint32_t source_ip, uint16_t source_port);

#endif