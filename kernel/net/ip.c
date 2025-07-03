#include <stdint.h>
#include <stddef.h>
#include "../core/kprintf.h"
#include "../core/mm.h"
#include "../drivers/net/rtl8139.h"
#include "arp.h"
#include "icmp.h"
#include "tcp.h"
#include "udp.h"

#define IP_VERSION_4 4
#define IP_HEADER_LENGTH 20
#define IP_MAX_PACKET_SIZE 65535
#define IP_DEFAULT_TTL 64

typedef struct {
    uint8_t version_ihl;
    uint8_t tos;
    uint16_t total_length;
    uint16_t id;
    uint16_t flags_fragment_offset;
    uint8_t ttl;
    uint8_t protocol;
    uint16_t header_checksum;
    uint32_t source_ip;
    uint32_t dest_ip;
} __attribute__((packed)) ip_header_t;

// IP configuration
static uint32_t local_ip = 0;
static uint32_t subnet_mask = 0;
static uint32_t gateway_ip = 0;
static uint16_t ip_id_counter = 0;

// IP statistics
typedef struct {
    uint32_t packets_sent;
    uint32_t packets_received;
    uint32_t packets_forwarded;
    uint32_t packets_dropped;
    uint32_t checksum_errors;
    uint32_t fragmentation_errors;
} ip_stats_t;

static ip_stats_t ip_stats;

void ip_init(void) {
    kprintf("Initializing IP stack...\n");
    
    // Initialize statistics
    memset(&ip_stats, 0, sizeof(ip_stats));
    
    // TODO: Get IP configuration from DHCP
    local_ip = 0xC0A80164;      // 192.168.1.100
    subnet_mask = 0xFFFFFF00;   // 255.255.255.0
    gateway_ip = 0xC0A80101;    // 192.168.1.1
    
    kprintf("IP address: %d.%d.%d.%d\n",
            (local_ip >> 24) & 0xFF,
            (local_ip >> 16) & 0xFF,
            (local_ip >> 8) & 0xFF,
            local_ip & 0xFF);
            
    kprintf("IP stack initialized\n");
}

static uint16_t ip_checksum(const void* data, size_t length) {
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

static int is_local_network(uint32_t ip_addr) {
    return (ip_addr & subnet_mask) == (local_ip & subnet_mask);
}

static uint32_t get_next_hop(uint32_t dest_ip) {
    if (is_local_network(dest_ip)) {
        return dest_ip;  // Direct delivery
    } else {
        return gateway_ip;  // Via gateway
    }
}

int ip_send_packet(uint32_t dest_ip, uint8_t protocol, const void* data, size_t length) {
    if (!data || length > IP_MAX_PACKET_SIZE - IP_HEADER_LENGTH) {
        return -1;
    }
    
    // Create IP header
    ip_header_t header;
    header.version_ihl = (IP_VERSION_4 << 4) | (IP_HEADER_LENGTH / 4);
    header.tos = 0;
    header.total_length = htons(IP_HEADER_LENGTH + length);
    header.id = htons(++ip_id_counter);
    header.flags_fragment_offset = htons(0x4000);  // Don't fragment
    header.ttl = IP_DEFAULT_TTL;
    header.protocol = protocol;
    header.header_checksum = 0;
    header.source_ip = htonl(local_ip);
    header.dest_ip = htonl(dest_ip);
    
    // Calculate checksum
    header.header_checksum = ip_checksum(&header, IP_HEADER_LENGTH);
    
    // Allocate packet buffer
    size_t packet_size = IP_HEADER_LENGTH + length;
    uint8_t* packet = (uint8_t*)kmalloc(packet_size);
    if (!packet) {
        return -1;
    }
    
    // Build packet
    memcpy(packet, &header, IP_HEADER_LENGTH);
    memcpy(packet + IP_HEADER_LENGTH, data, length);
    
    // Determine next hop
    uint32_t next_hop = get_next_hop(dest_ip);
    
    // Resolve MAC address via ARP
    uint8_t dest_mac[6];
    if (arp_resolve(next_hop, dest_mac) != 0) {
        kfree(packet);
        return -1;
    }
    
    // Send packet via Ethernet
    int result = rtl8139_send_packet(packet, packet_size);
    
    if (result == 0) {
        ip_stats.packets_sent++;
    }
    
    kfree(packet);
    return result;
}

void ip_handle_packet(const void* packet, size_t length) {
    if (!packet || length < IP_HEADER_LENGTH) {
        ip_stats.packets_dropped++;
        return;
    }
    
    const ip_header_t* header = (const ip_header_t*)packet;
    
    // Verify header
    if ((header->version_ihl >> 4) != IP_VERSION_4) {
        ip_stats.packets_dropped++;
        return;  // Not IPv4
    }
    
    uint8_t header_length = (header->version_ihl & 0x0F) * 4;
    if (header_length < IP_HEADER_LENGTH || length < header_length) {
        ip_stats.packets_dropped++;
        return;  // Invalid header
    }
    
    // Verify checksum
    uint16_t orig_checksum = header->header_checksum;
    ((ip_header_t*)header)->header_checksum = 0;
    if (ip_checksum(header, header_length) != orig_checksum) {
        ip_stats.checksum_errors++;
        ip_stats.packets_dropped++;
        return;  // Invalid checksum
    }
    
    uint32_t dest_ip = ntohl(header->dest_ip);
    uint32_t source_ip = ntohl(header->source_ip);
    
    // Check if packet is for us or broadcast
    if (dest_ip != local_ip && dest_ip != 0xFFFFFFFF && 
        dest_ip != (local_ip | ~subnet_mask)) {
        // Not for us - could implement forwarding here
        ip_stats.packets_dropped++;
        return;
    }
    
    ip_stats.packets_received++;
    
    // Handle fragmentation
    uint16_t flags_frag = ntohs(header->flags_fragment_offset);
    if ((flags_frag & 0x1FFF) != 0 || (flags_frag & 0x2000) != 0) {
        // Fragmented packet - not implemented yet
        ip_stats.fragmentation_errors++;
        ip_stats.packets_dropped++;
        return;
    }
    
    // Extract payload
    const void* payload = (const uint8_t*)packet + header_length;
    size_t payload_length = ntohs(header->total_length) - header_length;
    
    // Handle based on protocol
    switch (header->protocol) {
        case 1:  // ICMP
            icmp_handle_packet(payload, payload_length, source_ip);
            break;
            
        case 6:  // TCP
            tcp_handle_packet(payload, payload_length);
            break;
            
        case 17:  // UDP
            udp_handle_packet(payload, payload_length, source_ip);
            break;
            
        default:
            kprintf("Unknown IP protocol: %d\n", header->protocol);
            break;
    }
}

// Configuration functions
void ip_set_address(uint32_t ip, uint32_t mask, uint32_t gateway) {
    local_ip = ip;
    subnet_mask = mask;
    gateway_ip = gateway;
    
    kprintf("IP configuration updated:\n");
    kprintf("  IP: %d.%d.%d.%d\n",
            (ip >> 24) & 0xFF, (ip >> 16) & 0xFF,
            (ip >> 8) & 0xFF, ip & 0xFF);
    kprintf("  Mask: %d.%d.%d.%d\n",
            (mask >> 24) & 0xFF, (mask >> 16) & 0xFF,
            (mask >> 8) & 0xFF, mask & 0xFF);
    kprintf("  Gateway: %d.%d.%d.%d\n",
            (gateway >> 24) & 0xFF, (gateway >> 16) & 0xFF,
            (gateway >> 8) & 0xFF, gateway & 0xFF);
}

uint32_t ip_get_local_address(void) {
    return local_ip;
}

uint32_t ip_get_subnet_mask(void) {
    return subnet_mask;
}

uint32_t ip_get_gateway_address(void) {
    return gateway_ip;
}

void ip_get_stats(ip_stats_t* stats) {
    if (stats) {
        *stats = ip_stats;
    }
}

// Utility functions
const char* ip_to_string(uint32_t ip) {
    static char buffer[16];
    snprintf(buffer, sizeof(buffer), "%d.%d.%d.%d",
             (ip >> 24) & 0xFF, (ip >> 16) & 0xFF,
             (ip >> 8) & 0xFF, ip & 0xFF);
    return buffer;
}

uint32_t string_to_ip(const char* str) {
    if (!str) return 0;
    
    uint32_t a, b, c, d;
    if (sscanf(str, "%u.%u.%u.%u", &a, &b, &c, &d) != 4) {
        return 0;
    }
    
    if (a > 255 || b > 255 || c > 255 || d > 255) {
        return 0;
    }
    
    return (a << 24) | (b << 16) | (c << 8) | d;
}