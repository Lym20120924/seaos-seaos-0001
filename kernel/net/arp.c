#include <stdint.h>
#include <stddef.h>
#include "../core/kprintf.h"
#include "../core/mm.h"
#include "arp.h"
#include "../drivers/net/rtl8139.h"

#define ARP_CACHE_SIZE 128
#define ARP_TIMEOUT    300  // 5 minutes in seconds
#define ARP_RETRY      3    // Number of retries
#define ARP_WAIT       1000 // Wait time in milliseconds

// ARP packet structure
typedef struct {
    uint16_t hw_type;
    uint16_t proto_type;
    uint8_t  hw_len;
    uint8_t  proto_len;
    uint16_t operation;
    uint8_t  sender_mac[6];
    uint32_t sender_ip;
    uint8_t  target_mac[6];
    uint32_t target_ip;
} __attribute__((packed)) arp_packet_t;

// ARP cache entry
typedef struct {
    uint32_t ip_addr;
    uint8_t  mac_addr[6];
    uint8_t  state;
    uint32_t timestamp;
} arp_cache_entry_t;

static arp_cache_entry_t arp_cache[ARP_CACHE_SIZE];
static uint8_t local_mac[6];
static uint32_t local_ip;

void arp_init(void) {
    kprintf("Initializing ARP protocol...\n");
    
    // Initialize cache
    memset(arp_cache, 0, sizeof(arp_cache));
    
    // TODO: Get local MAC and IP addresses
    // For now, use example values
    memcpy(local_mac, "\x52\x54\x00\x12\x34\x56", 6);
    local_ip = 0xC0A80164; // 192.168.1.100
    
    kprintf("ARP protocol initialized\n");
}

static void send_arp_request(uint32_t target_ip) {
    arp_packet_t packet;
    
    // Fill ARP packet
    packet.hw_type = htons(1);        // Ethernet
    packet.proto_type = htons(0x0800); // IPv4
    packet.hw_len = 6;                // MAC address length
    packet.proto_len = 4;             // IPv4 address length
    packet.operation = htons(ARP_REQUEST);
    
    memcpy(packet.sender_mac, local_mac, 6);
    packet.sender_ip = local_ip;
    memset(packet.target_mac, 0, 6);
    packet.target_ip = target_ip;
    
    // Send packet
    rtl8139_send_packet(&packet, sizeof(packet));
}

static void send_arp_reply(const arp_packet_t* request) {
    arp_packet_t reply;
    
    // Fill ARP packet
    reply.hw_type = htons(1);
    reply.proto_type = htons(0x0800);
    reply.hw_len = 6;
    reply.proto_len = 4;
    reply.operation = htons(ARP_REPLY);
    
    memcpy(reply.sender_mac, local_mac, 6);
    reply.sender_ip = local_ip;
    memcpy(reply.target_mac, request->sender_mac, 6);
    reply.target_ip = request->sender_ip;
    
    // Send packet
    rtl8139_send_packet(&reply, sizeof(reply));
}

int arp_resolve(uint32_t ip_addr, uint8_t* mac_addr) {
    if (!mac_addr) {
        return -1;
    }
    
    // Check cache first
    if (arp_cache_lookup(ip_addr, mac_addr) == 0) {
        return 0;
    }
    
    // Send ARP request
    for (int retry = 0; retry < ARP_RETRY; retry++) {
        send_arp_request(ip_addr);
        
        // Wait for reply
        // TODO: Implement proper waiting mechanism
        for (int i = 0; i < ARP_WAIT; i++) {
            if (arp_cache_lookup(ip_addr, mac_addr) == 0) {
                return 0;
            }
        }
    }
    
    return -1;
}

void arp_update(uint32_t ip_addr, const uint8_t* mac_addr) {
    if (!mac_addr) {
        return;
    }
    
    // Find existing or free entry
    arp_cache_entry_t* entry = NULL;
    arp_cache_entry_t* free_entry = NULL;
    uint32_t oldest_time = 0xFFFFFFFF;
    arp_cache_entry_t* oldest_entry = NULL;
    
    for (int i = 0; i < ARP_CACHE_SIZE; i++) {
        if (arp_cache[i].state == ARP_STATE_FREE) {
            if (!free_entry) {
                free_entry = &arp_cache[i];
            }
        } else if (arp_cache[i].ip_addr == ip_addr) {
            entry = &arp_cache[i];
            break;
        } else if (arp_cache[i].timestamp < oldest_time) {
            oldest_time = arp_cache[i].timestamp;
            oldest_entry = &arp_cache[i];
        }
    }
    
    // Use existing entry or allocate new one
    if (!entry) {
        entry = free_entry ? free_entry : oldest_entry;
        if (!entry) {
            return;
        }
    }
    
    // Update entry
    entry->ip_addr = ip_addr;
    memcpy(entry->mac_addr, mac_addr, 6);
    entry->state = ARP_STATE_RESOLVED;
    entry->timestamp = 0; // TODO: Get current time
}

void arp_handle_packet(const void* packet, size_t length) {
    if (!packet || length < sizeof(arp_packet_t)) {
        return;
    }
    
    const arp_packet_t* arp = (const arp_packet_t*)packet;
    
    // Verify packet
    if (ntohs(arp->hw_type) != 1 ||      // Ethernet
        ntohs(arp->proto_type) != 0x0800 || // IPv4
        arp->hw_len != 6 ||              // MAC address length
        arp->proto_len != 4) {           // IPv4 address length
        return;
    }
    
    // Update cache with sender's information
    arp_update(arp->sender_ip, arp->sender_mac);
    
    // Handle ARP request
    if (ntohs(arp->operation) == ARP_REQUEST &&
        arp->target_ip == local_ip) {
        send_arp_reply(arp);
    }
}

void arp_cache_clear(void) {
    memset(arp_cache, 0, sizeof(arp_cache));
}

int arp_cache_lookup(uint32_t ip_addr, uint8_t* mac_addr) {
    if (!mac_addr) {
        return -1;
    }
    
    uint32_t current_time = 0; // TODO: Get current time
    
    for (int i = 0; i < ARP_CACHE_SIZE; i++) {
        if (arp_cache[i].state == ARP_STATE_RESOLVED &&
            arp_cache[i].ip_addr == ip_addr) {
            // Check if entry has expired
            if (current_time - arp_cache[i].timestamp > ARP_TIMEOUT) {
                arp_cache[i].state = ARP_STATE_FREE;
                return -1;
            }
            
            memcpy(mac_addr, arp_cache[i].mac_addr, 6);
            return 0;
        }
    }
    
    return -1;
}