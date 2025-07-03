#include <stdint.h>
#include <stddef.h>
#include "../core/kprintf.h"
#include "../core/mm.h"
#include "udp.h"
#include "dhcp.h"
#include "ip.h"

#define DHCP_SERVER_PORT 67
#define DHCP_CLIENT_PORT 68
#define DHCP_MAGIC_COOKIE 0x63825363
#define DHCP_MAX_PACKET_SIZE 576

// DHCP message structure
typedef struct {
    uint8_t op;
    uint8_t htype;
    uint8_t hlen;
    uint8_t hops;
    uint32_t xid;
    uint16_t secs;
    uint16_t flags;
    uint32_t ciaddr;
    uint32_t yiaddr;
    uint32_t siaddr;
    uint32_t giaddr;
    uint8_t chaddr[16];
    uint8_t sname[64];
    uint8_t file[128];
    uint32_t magic_cookie;
    uint8_t options[];
} __attribute__((packed)) dhcp_message_t;

// DHCP client states
typedef enum {
    DHCP_STATE_INIT,
    DHCP_STATE_SELECTING,
    DHCP_STATE_REQUESTING,
    DHCP_STATE_BOUND,
    DHCP_STATE_RENEWING,
    DHCP_STATE_REBINDING
} dhcp_state_t;

static dhcp_state_t dhcp_state = DHCP_STATE_INIT;
static uint32_t transaction_id = 0;
static dhcp_config_t current_config;
static uint64_t lease_start_time = 0;
static uint32_t server_identifier = 0;
static uint8_t client_mac[6] = {0x52, 0x54, 0x00, 0x12, 0x34, 0x56}; // Example MAC

void dhcp_init(void) {
    kprintf("Initializing DHCP client...\n");
    
    // Register UDP listener for DHCP responses
    udp_register_listener(DHCP_CLIENT_PORT, dhcp_handle_packet);
    
    // Initialize configuration
    memset(&current_config, 0, sizeof(current_config));
    
    kprintf("DHCP client initialized\n");
}

static void dhcp_add_option(uint8_t** options, uint8_t code, uint8_t length, const void* data) {
    **options = code;
    (*options)++;
    **options = length;
    (*options)++;
    if (data && length > 0) {
        memcpy(*options, data, length);
        *options += length;
    }
}

static void send_dhcp_discover(void) {
    uint8_t buffer[DHCP_MAX_PACKET_SIZE];
    dhcp_message_t* message = (dhcp_message_t*)buffer;
    
    // Initialize message
    memset(message, 0, sizeof(dhcp_message_t));
    message->op = 1; // BOOTREQUEST
    message->htype = 1; // Ethernet
    message->hlen = 6; // MAC address length
    message->xid = htonl(++transaction_id);
    message->secs = 0;
    message->flags = htons(0x8000); // Broadcast
    memcpy(message->chaddr, client_mac, 6);
    message->magic_cookie = htonl(DHCP_MAGIC_COOKIE);
    
    // Add options
    uint8_t* options = message->options;
    
    // Message type
    dhcp_add_option(&options, 53, 1, &(uint8_t){DHCP_DISCOVER});
    
    // Parameter request list
    uint8_t param_list[] = {1, 3, 6, 51}; // Subnet mask, Router, DNS, Lease time
    dhcp_add_option(&options, 55, sizeof(param_list), param_list);
    
    // Client identifier
    uint8_t client_id[7] = {1}; // Hardware type
    memcpy(client_id + 1, client_mac, 6);
    dhcp_add_option(&options, 61, sizeof(client_id), client_id);
    
    // End option
    *options++ = 255;
    
    // Send packet
    size_t packet_size = options - buffer;
    udp_send_packet(0xFFFFFFFF, DHCP_SERVER_PORT, DHCP_CLIENT_PORT, buffer, packet_size);
    
    dhcp_state = DHCP_STATE_SELECTING;
    kprintf("DHCP: Sent DISCOVER\n");
}

static void send_dhcp_request(uint32_t server_ip, uint32_t requested_ip) {
    uint8_t buffer[DHCP_MAX_PACKET_SIZE];
    dhcp_message_t* message = (dhcp_message_t*)buffer;
    
    // Initialize message
    memset(message, 0, sizeof(dhcp_message_t));
    message->op = 1; // BOOTREQUEST
    message->htype = 1;
    message->hlen = 6;
    message->xid = htonl(transaction_id);
    message->secs = 0;
    message->flags = htons(0x8000);
    memcpy(message->chaddr, client_mac, 6);
    message->magic_cookie = htonl(DHCP_MAGIC_COOKIE);
    
    // Add options
    uint8_t* options = message->options;
    
    // Message type
    dhcp_add_option(&options, 53, 1, &(uint8_t){DHCP_REQUEST});
    
    // Requested IP address
    uint32_t req_ip = htonl(requested_ip);
    dhcp_add_option(&options, 50, 4, &req_ip);
    
    // Server identifier
    uint32_t srv_id = htonl(server_ip);
    dhcp_add_option(&options, 54, 4, &srv_id);
    
    // Parameter request list
    uint8_t param_list[] = {1, 3, 6, 51};
    dhcp_add_option(&options, 55, sizeof(param_list), param_list);
    
    // Client identifier
    uint8_t client_id[7] = {1};
    memcpy(client_id + 1, client_mac, 6);
    dhcp_add_option(&options, 61, sizeof(client_id), client_id);
    
    // End option
    *options++ = 255;
    
    // Send packet
    size_t packet_size = options - buffer;
    udp_send_packet(0xFFFFFFFF, DHCP_SERVER_PORT, DHCP_CLIENT_PORT, buffer, packet_size);
    
    dhcp_state = DHCP_STATE_REQUESTING;
    kprintf("DHCP: Sent REQUEST for IP %d.%d.%d.%d\n",
            (requested_ip >> 24) & 0xFF, (requested_ip >> 16) & 0xFF,
            (requested_ip >> 8) & 0xFF, requested_ip & 0xFF);
}

static void send_dhcp_release(void) {
    if (dhcp_state != DHCP_STATE_BOUND) {
        return;
    }
    
    uint8_t buffer[DHCP_MAX_PACKET_SIZE];
    dhcp_message_t* message = (dhcp_message_t*)buffer;
    
    // Initialize message
    memset(message, 0, sizeof(dhcp_message_t));
    message->op = 1;
    message->htype = 1;
    message->hlen = 6;
    message->xid = htonl(++transaction_id);
    message->ciaddr = htonl(current_config.ip_address);
    memcpy(message->chaddr, client_mac, 6);
    message->magic_cookie = htonl(DHCP_MAGIC_COOKIE);
    
    // Add options
    uint8_t* options = message->options;
    
    // Message type
    dhcp_add_option(&options, 53, 1, &(uint8_t){DHCP_RELEASE});
    
    // Server identifier
    uint32_t srv_id = htonl(server_identifier);
    dhcp_add_option(&options, 54, 4, &srv_id);
    
    // Client identifier
    uint8_t client_id[7] = {1};
    memcpy(client_id + 1, client_mac, 6);
    dhcp_add_option(&options, 61, sizeof(client_id), client_id);
    
    // End option
    *options++ = 255;
    
    // Send packet
    size_t packet_size = options - buffer;
    udp_send_packet(server_identifier, DHCP_SERVER_PORT, DHCP_CLIENT_PORT, buffer, packet_size);
    
    kprintf("DHCP: Sent RELEASE\n");
}

int dhcp_start(void) {
    if (dhcp_state != DHCP_STATE_INIT) {
        return -1;
    }
    
    send_dhcp_discover();
    return 0;
}

void dhcp_release(void) {
    send_dhcp_release();
    
    // Reset state
    dhcp_state = DHCP_STATE_INIT;
    memset(&current_config, 0, sizeof(current_config));
    server_identifier = 0;
}

void dhcp_handle_packet(const void* packet, size_t length, uint32_t source_ip, uint16_t source_port) {
    if (!packet || length < sizeof(dhcp_message_t)) {
        return;
    }
    
    const dhcp_message_t* message = (const dhcp_message_t*)packet;
    
    // Verify message
    if (message->op != 2 || // BOOTREPLY
        message->magic_cookie != htonl(DHCP_MAGIC_COOKIE) ||
        ntohl(message->xid) != transaction_id) {
        return;
    }
    
    // Parse options
    const uint8_t* options = message->options;
    const uint8_t* end = (const uint8_t*)packet + length;
    uint8_t msg_type = 0;
    uint32_t server_id = 0;
    
    while (options < end && *options != 255) {
        uint8_t option = *options++;
        if (options >= end) break;
        
        uint8_t len = *options++;
        if (options + len > end) break;
        
        switch (option) {
            case 53: // Message type
                if (len >= 1) {
                    msg_type = *options;
                }
                break;
                
            case 54: // Server identifier
                if (len >= 4) {
                    server_id = ntohl(*(uint32_t*)options);
                }
                break;
                
            case 1: // Subnet mask
                if (len >= 4) {
                    current_config.subnet_mask = ntohl(*(uint32_t*)options);
                }
                break;
                
            case 3: // Router
                if (len >= 4) {
                    current_config.gateway = ntohl(*(uint32_t*)options);
                }
                break;
                
            case 6: // DNS server
                if (len >= 4) {
                    current_config.dns_server = ntohl(*(uint32_t*)options);
                }
                break;
                
            case 51: // Lease time
                if (len >= 4) {
                    current_config.lease_time = ntohl(*(uint32_t*)options);
                }
                break;
        }
        
        options += len;
    }
    
    // Handle message based on type
    switch (msg_type) {
        case DHCP_OFFER:
            if (dhcp_state == DHCP_STATE_SELECTING) {
                server_identifier = server_id;
                send_dhcp_request(server_id, ntohl(message->yiaddr));
            }
            break;
            
        case DHCP_ACK:
            if (dhcp_state == DHCP_STATE_REQUESTING) {
                current_config.ip_address = ntohl(message->yiaddr);
                lease_start_time = 0; // TODO: Get current time
                dhcp_state = DHCP_STATE_BOUND;
                
                // Configure IP stack
                ip_set_address(current_config.ip_address, 
                             current_config.subnet_mask,
                             current_config.gateway);
                
                kprintf("DHCP: Bound to IP %d.%d.%d.%d\n",
                        (current_config.ip_address >> 24) & 0xFF,
                        (current_config.ip_address >> 16) & 0xFF,
                        (current_config.ip_address >> 8) & 0xFF,
                        current_config.ip_address & 0xFF);
                kprintf("DHCP: Gateway: %d.%d.%d.%d\n",
                        (current_config.gateway >> 24) & 0xFF,
                        (current_config.gateway >> 16) & 0xFF,
                        (current_config.gateway >> 8) & 0xFF,
                        current_config.gateway & 0xFF);
                kprintf("DHCP: DNS: %d.%d.%d.%d\n",
                        (current_config.dns_server >> 24) & 0xFF,
                        (current_config.dns_server >> 16) & 0xFF,
                        (current_config.dns_server >> 8) & 0xFF,
                        current_config.dns_server & 0xFF);
            }
            break;
            
        case DHCP_NAK:
            if (dhcp_state == DHCP_STATE_REQUESTING ||
                dhcp_state == DHCP_STATE_RENEWING ||
                dhcp_state == DHCP_STATE_REBINDING) {
                dhcp_state = DHCP_STATE_INIT;
                memset(&current_config, 0, sizeof(current_config));
                server_identifier = 0;
                send_dhcp_discover();
            }
            break;
    }
}

int dhcp_is_bound(void) {
    return dhcp_state == DHCP_STATE_BOUND;
}

const dhcp_config_t* dhcp_get_config(void) {
    return dhcp_state == DHCP_STATE_BOUND ? &current_config : NULL;
}

// Renew lease (should be called periodically)
void dhcp_renew(void) {
    if (dhcp_state == DHCP_STATE_BOUND) {
        // TODO: Check if renewal time has passed
        // For now, just send a renewal request
        dhcp_state = DHCP_STATE_RENEWING;
        send_dhcp_request(server_identifier, current_config.ip_address);
    }
}

// Get DHCP state
int dhcp_get_state(void) {
    return dhcp_state;
}