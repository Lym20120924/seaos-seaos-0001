#include <stdint.h>
#include <stddef.h>
#include "../core/kprintf.h"
#include "../core/mm.h"
#include "ip.h"
#include "udp.h"

#define UDP_MAX_PACKET_SIZE 65507  // Max UDP packet size (65535 - IP header - UDP header)

typedef struct {
    uint16_t source_port;
    uint16_t dest_port;
    uint16_t length;
    uint16_t checksum;
} __attribute__((packed)) udp_header_t;

typedef struct {
    uint16_t port;
    void (*handler)(const void* data, size_t length, uint32_t source_ip, uint16_t source_port);
} udp_listener_t;

#define MAX_UDP_LISTENERS 32
static udp_listener_t listeners[MAX_UDP_LISTENERS];
static int num_listeners = 0;

void udp_init(void) {
    kprintf("Initializing UDP stack...\n");
    num_listeners = 0;
    kprintf("UDP stack initialized\n");
}

static uint16_t udp_checksum(const udp_header_t* header, const void* data, size_t length,
                           uint32_t source_ip, uint32_t dest_ip) {
    uint32_t sum = 0;
    
    // Add pseudo-header
    sum += (source_ip >> 16) & 0xFFFF;
    sum += source_ip & 0xFFFF;
    sum += (dest_ip >> 16) & 0xFFFF;
    sum += dest_ip & 0xFFFF;
    sum += htons(17); // Protocol (UDP)
    sum += header->length;
    
    // Add UDP header
    sum += header->source_port;
    sum += header->dest_port;
    sum += header->length;
    // Skip checksum field
    
    // Add data
    const uint16_t* ptr = (const uint16_t*)data;
    while (length > 1) {
        sum += *ptr++;
        length -= 2;
    }
    
    if (length > 0) {
        sum += *(uint8_t*)ptr;
    }
    
    // Fold 32-bit sum into 16 bits
    while (sum >> 16) {
        sum = (sum & 0xFFFF) + (sum >> 16);
    }
    
    return ~sum;
}

int udp_register_listener(uint16_t port, 
    void (*handler)(const void* data, size_t length, uint32_t source_ip, uint16_t source_port)) {
    if (num_listeners >= MAX_UDP_LISTENERS || !handler) {
        return -1;
    }
    
    // Check for existing listener
    for (int i = 0; i < num_listeners; i++) {
        if (listeners[i].port == port) {
            return -1; // Port already in use
        }
    }
    
    listeners[num_listeners].port = port;
    listeners[num_listeners].handler = handler;
    num_listeners++;
    
    kprintf("UDP listener registered on port %d\n", port);
    return 0;
}

void udp_unregister_listener(uint16_t port) {
    for (int i = 0; i < num_listeners; i++) {
        if (listeners[i].port == port) {
            // Remove listener by shifting remaining ones
            for (int j = i; j < num_listeners - 1; j++) {
                listeners[j] = listeners[j + 1];
            }
            num_listeners--;
            kprintf("UDP listener unregistered from port %d\n", port);
            break;
        }
    }
}

int udp_send_packet(uint32_t dest_ip, uint16_t dest_port, uint16_t source_port, 
                   const void* data, size_t length) {
    if (!data || length > UDP_MAX_PACKET_SIZE) {
        return -1;
    }
    
    // Create UDP header
    udp_header_t header;
    header.source_port = htons(source_port);
    header.dest_port = htons(dest_port);
    header.length = htons(sizeof(udp_header_t) + length);
    header.checksum = 0;
    
    // Calculate checksum
    uint32_t local_ip = ip_get_local_address();
    header.checksum = udp_checksum(&header, data, length, local_ip, dest_ip);
    
    // Allocate packet buffer
    size_t packet_size = sizeof(udp_header_t) + length;
    uint8_t* packet = (uint8_t*)kmalloc(packet_size);
    if (!packet) {
        return -1;
    }
    
    // Build packet
    memcpy(packet, &header, sizeof(udp_header_t));
    if (length > 0) {
        memcpy(packet + sizeof(udp_header_t), data, length);
    }
    
    // Send via IP
    int result = ip_send_packet(dest_ip, 17, packet, packet_size);
    
    kfree(packet);
    return result;
}

void udp_handle_packet(const void* packet, size_t length, uint32_t source_ip) {
    if (!packet || length < sizeof(udp_header_t)) {
        return;
    }
    
    const udp_header_t* header = (const udp_header_t*)packet;
    const void* data = (const uint8_t*)packet + sizeof(udp_header_t);
    size_t data_length = length - sizeof(udp_header_t);
    
    // Verify packet length
    if (ntohs(header->length) != length) {
        return;
    }
    
    // Verify checksum (if not zero)
    if (header->checksum != 0) {
        uint16_t orig_checksum = header->checksum;
        ((udp_header_t*)header)->checksum = 0;
        uint32_t local_ip = ip_get_local_address();
        if (udp_checksum(header, data, data_length, source_ip, local_ip) != orig_checksum) {
            return;
        }
    }
    
    // Find matching listener
    uint16_t dest_port = ntohs(header->dest_port);
    uint16_t source_port = ntohs(header->source_port);
    
    for (int i = 0; i < num_listeners; i++) {
        if (listeners[i].port == dest_port) {
            listeners[i].handler(data, data_length, source_ip, source_port);
            return;
        }
    }
    
    // No listener found - could send ICMP port unreachable
    kprintf("UDP packet received for unregistered port %d\n", dest_port);
}

// Utility functions for common UDP operations
int udp_send_broadcast(uint16_t port, uint16_t source_port, const void* data, size_t length) {
    return udp_send_packet(0xFFFFFFFF, port, source_port, data, length);
}

int udp_send_multicast(uint32_t multicast_ip, uint16_t port, uint16_t source_port, 
                      const void* data, size_t length) {
    // Verify multicast address (224.0.0.0 to 239.255.255.255)
    if ((multicast_ip & 0xF0000000) != 0xE0000000) {
        return -1;
    }
    
    return udp_send_packet(multicast_ip, port, source_port, data, length);
}

// Get statistics
void udp_get_stats(udp_stats_t* stats) {
    if (!stats) return;
    
    // TODO: Implement statistics tracking
    memset(stats, 0, sizeof(udp_stats_t));
    stats->listeners_count = num_listeners;
}