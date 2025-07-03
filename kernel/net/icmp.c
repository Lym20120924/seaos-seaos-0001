#include <stdint.h>
#include <stddef.h>
#include "../core/kprintf.h"
#include "../core/mm.h"
#include "ip.h"
#include "icmp.h"

typedef struct {
    uint8_t type;
    uint8_t code;
    uint16_t checksum;
    union {
        struct {
            uint16_t identifier;
            uint16_t sequence;
        } echo;
        uint32_t gateway;
        struct {
            uint16_t unused;
            uint16_t mtu;
        } frag;
    } un;
    uint8_t data[];
} icmp_header_t;

static icmp_callback_t callback = NULL;

void icmp_init(void) {
    kprintf("Initializing ICMP protocol...\n");
    callback = NULL;
    kprintf("ICMP protocol initialized\n");
}

static uint16_t icmp_checksum(const void* data, size_t length) {
    const uint16_t* ptr = (const uint16_t*)data;
    uint32_t sum = 0;
    
    while (length > 1) {
        sum += *ptr++;
        length -= 2;
    }
    
    if (length > 0) {
        sum += *(uint8_t*)ptr;
    }
    
    while (sum >> 16) {
        sum = (sum & 0xFFFF) + (sum >> 16);
    }
    
    return ~sum;
}

int icmp_send_echo_request(uint32_t dest_ip, uint16_t sequence, uint16_t identifier) {
    // Create ICMP packet
    size_t packet_size = sizeof(icmp_header_t) + 56;  // Standard ping size
    uint8_t* packet = (uint8_t*)kmalloc(packet_size);
    if (!packet) {
        return -1;
    }
    
    // Initialize header
    icmp_header_t* header = (icmp_header_t*)packet;
    header->type = ICMP_ECHO_REQUEST;
    header->code = 0;
    header->checksum = 0;
    header->un.echo.identifier = identifier;
    header->un.echo.sequence = sequence;
    
    // Fill data portion with pattern
    for (int i = 0; i < 56; i++) {
        header->data[i] = i & 0xFF;
    }
    
    // Calculate checksum
    header->checksum = icmp_checksum(packet, packet_size);
    
    // Send via IP
    int result = ip_send_packet(dest_ip, 1, packet, packet_size);
    
    kfree(packet);
    return result;
}

void icmp_handle_packet(const void* packet, size_t length, uint32_t source_ip) {
    if (!packet || length < sizeof(icmp_header_t)) {
        return;
    }
    
    const icmp_header_t* header = (const icmp_header_t*)packet;
    
    // Verify checksum
    uint16_t orig_checksum = header->checksum;
    ((icmp_header_t*)header)->checksum = 0;
    if (icmp_checksum(packet, length) != orig_checksum) {
        return;
    }
    
    // Handle echo request
    if (header->type == ICMP_ECHO_REQUEST) {
        // Create response packet
        uint8_t* response = (uint8_t*)kmalloc(length);
        if (!response) {
            return;
        }
        
        // Copy original packet
        memcpy(response, packet, length);
        icmp_header_t* resp_header = (icmp_header_t*)response;
        
        // Modify for response
        resp_header->type = ICMP_ECHO_REPLY;
        resp_header->checksum = 0;
        resp_header->checksum = icmp_checksum(response, length);
        
        // Send response
        ip_send_packet(source_ip, 1, response, length);
        
        kfree(response);
    }
    
    // Notify callback if registered
    if (callback) {
        const void* data = header->data;
        size_t data_length = length - sizeof(icmp_header_t);
        callback(header->type, header->code, data, data_length, source_ip);
    }
}

void icmp_register_callback(icmp_callback_t cb) {
    callback = cb;
}