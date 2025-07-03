#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "../core/kprintf.h"
#include "../core/mm.h"
#include "udp.h"
#include "dns.h"

#define DNS_PORT 53
#define DNS_MAX_PACKET_SIZE 512
#define DNS_MAX_LABEL_SIZE 63
#define DNS_MAX_NAME_SIZE 255
#define DNS_CACHE_SIZE 64
#define DNS_QUERY_TIMEOUT 5000  // 5 seconds

typedef struct {
    uint16_t id;
    uint16_t flags;
    uint16_t qdcount;
    uint16_t ancount;
    uint16_t nscount;
    uint16_t arcount;
} dns_header_t;

typedef struct {
    char* hostname;
    uint32_t ip_addr;
    uint32_t ttl;
    uint64_t expire_time;
} dns_cache_entry_t;

static uint32_t dns_nameserver = 0;
static uint16_t dns_query_id = 0;
static dns_cache_entry_t dns_cache[DNS_CACHE_SIZE];
static size_t dns_cache_count = 0;

void dns_init(void) {
    kprintf("Initializing DNS resolver...\n");
    
    // Initialize cache
    memset(dns_cache, 0, sizeof(dns_cache));
    dns_cache_count = 0;
    
    // Register UDP listener for DNS responses
    udp_register_listener(DNS_PORT, dns_handle_response);
    
    // Set default nameserver (8.8.8.8)
    dns_nameserver = 0x08080808;
    
    kprintf("DNS resolver initialized\n");
}

static int dns_encode_name(const char* name, uint8_t* buffer) {
    size_t total_size = 0;
    size_t label_size = 0;
    uint8_t* label_length = buffer++;
    total_size++;
    
    while (*name) {
        if (*name == '.') {
            if (label_size > DNS_MAX_LABEL_SIZE) {
                return -1;
            }
            *label_length = label_size;
            label_length = buffer++;
            total_size++;
            label_size = 0;
        } else {
            *buffer++ = *name;
            label_size++;
            total_size++;
        }
        name++;
    }
    
    if (label_size > 0) {
        if (label_size > DNS_MAX_LABEL_SIZE) {
            return -1;
        }
        *label_length = label_size;
        *buffer++ = 0;
        total_size++;
    }
    
    return total_size;
}

static int dns_decode_name(const uint8_t* packet, size_t packet_size, 
                         size_t offset, char* name, size_t name_size) {
    size_t name_offset = 0;
    size_t current_offset = offset;
    int jumped = 0;
    
    while (current_offset < packet_size) {
        uint8_t length = packet[current_offset++];
        
        if (length == 0) {
            break;
        }
        
        // Handle compression
        if ((length & 0xC0) == 0xC0) {
            if (!jumped) {
                offset = current_offset + 1;
            }
            
            uint16_t jump_offset = ((length & 0x3F) << 8) | packet[current_offset];
            current_offset = jump_offset;
            jumped = 1;
            
            continue;
        }
        
        // Copy label
        if (name_offset + length + 1 > name_size) {
            return -1;
        }
        
        memcpy(name + name_offset, &packet[current_offset], length);
        name_offset += length;
        name[name_offset++] = '.';
        
        current_offset += length;
    }
    
    if (name_offset > 0) {
        name[name_offset - 1] = '\0';
    } else {
        name[0] = '\0';
    }
    
    return jumped ? offset : current_offset;
}

int dns_resolve(const char* hostname, uint32_t* ip_addr) {
    if (!hostname || !ip_addr) {
        return -1;
    }
    
    // Check cache first
    if (dns_cache_lookup(hostname, ip_addr) == 0) {
        return 0;
    }
    
    // Prepare DNS query
    uint8_t packet[DNS_MAX_PACKET_SIZE];
    dns_header_t* header = (dns_header_t*)packet;
    uint8_t* query = packet + sizeof(dns_header_t);
    
    // Initialize header
    header->id = ++dns_query_id;
    header->flags = htons(DNS_RD_FLAG);
    header->qdcount = htons(1);
    header->ancount = 0;
    header->nscount = 0;
    header->arcount = 0;
    
    // Encode query name
    int name_size = dns_encode_name(hostname, query);
    if (name_size < 0) {
        return -1;
    }
    query += name_size;
    
    // Add query type and class
    *(uint16_t*)query = htons(DNS_TYPE_A);
    query += 2;
    *(uint16_t*)query = htons(DNS_CLASS_IN);
    query += 2;
    
    // Send query
    size_t packet_size = query - packet;
    return udp_send_packet(dns_nameserver, DNS_PORT, DNS_PORT, packet, packet_size);
}

void dns_handle_response(const void* packet, size_t length) {
    if (!packet || length < sizeof(dns_header_t)) {
        return;
    }
    
    const dns_header_t* header = (const dns_header_t*)packet;
    const uint8_t* data = (const uint8_t*)packet + sizeof(dns_header_t);
    
    // Verify response
    if ((ntohs(header->flags) & DNS_QR_RESPONSE) == 0) {
        return;  // Not a response
    }
    
    uint16_t rcode = ntohs(header->flags) & DNS_RCODE_MASK;
    if (rcode != 0) {
        return;  // Error response
    }
    
    // Skip query section
    char name[DNS_MAX_NAME_SIZE];
    int offset = dns_decode_name(packet, length, sizeof(dns_header_t), name, sizeof(name));
    if (offset < 0) {
        return;
    }
    
    offset += 4;  // Skip query type and class
    
    // Process answers
    uint16_t ancount = ntohs(header->ancount);
    for (uint16_t i = 0; i < ancount; i++) {
        // Decode answer name
        offset = dns_decode_name(packet, length, offset, name, sizeof(name));
        if (offset < 0) {
            break;
        }
        
        // Parse answer fields
        if (offset + 10 > length) {
            break;
        }
        
        uint16_t type = ntohs(*(uint16_t*)&data[offset]);
        offset += 2;
        uint16_t class = ntohs(*(uint16_t*)&data[offset]);
        offset += 2;
        uint32_t ttl = ntohl(*(uint32_t*)&data[offset]);
        offset += 4;
        uint16_t rdlength = ntohs(*(uint16_t*)&data[offset]);
        offset += 2;
        
        if (offset + rdlength > length) {
            break;
        }
        
        // Handle A record
        if (type == DNS_TYPE_A && class == DNS_CLASS_IN && rdlength == 4) {
            uint32_t ip_addr = *(uint32_t*)&data[offset];
            dns_cache_add(name, ip_addr, ttl);
        }
        
        offset += rdlength;
    }
}

int dns_set_nameserver(uint32_t nameserver_ip) {
    dns_nameserver = nameserver_ip;
    return 0;
}

void dns_cache_clear(void) {
    for (size_t i = 0; i < dns_cache_count; i++) {
        if (dns_cache[i].hostname) {
            kfree(dns_cache[i].hostname);
        }
    }
    dns_cache_count = 0;
}

int dns_cache_lookup(const char* hostname, uint32_t* ip_addr) {
    uint64_t current_time = 0;  // TODO: Get current time
    
    for (size_t i = 0; i < dns_cache_count; i++) {
        if (dns_cache[i].hostname && 
            strcmp(dns_cache[i].hostname, hostname) == 0) {
            if (current_time < dns_cache[i].expire_time) {
                *ip_addr = dns_cache[i].ip_addr;
                return 0;
            }
            
            // Entry expired, remove it
            kfree(dns_cache[i].hostname);
            if (i < dns_cache_count - 1) {
                memmove(&dns_cache[i], &dns_cache[i + 1], 
                       (dns_cache_count - i - 1) * sizeof(dns_cache_entry_t));
            }
            dns_cache_count--;
            break;
        }
    }
    
    return -1;
}

int dns_cache_add(const char* hostname, uint32_t ip_addr, uint32_t ttl) {
    // Remove existing entry if present
    for (size_t i = 0; i < dns_cache_count; i++) {
        if (dns_cache[i].hostname && 
            strcmp(dns_cache[i].hostname, hostname) == 0) {
            kfree(dns_cache[i].hostname);
            if (i < dns_cache_count - 1) {
                memmove(&dns_cache[i], &dns_cache[i + 1], 
                       (dns_cache_count - i - 1) * sizeof(dns_cache_entry_t));
            }
            dns_cache_count--;
            break;
        }
    }
    
    // Add new entry
    if (dns_cache_count >= DNS_CACHE_SIZE) {
        // Cache full, remove oldest entry
        kfree(dns_cache[0].hostname);
        memmove(&dns_cache[0], &dns_cache[1], 
               (DNS_CACHE_SIZE - 1) * sizeof(dns_cache_entry_t));
        dns_cache_count--;
    }
    
    size_t hostname_len = strlen(hostname) + 1;
    dns_cache[dns_cache_count].hostname = (char*)kmalloc(hostname_len);
    if (!dns_cache[dns_cache_count].hostname) {
        return -1;
    }
    
    memcpy(dns_cache[dns_cache_count].hostname, hostname, hostname_len);
    dns_cache[dns_cache_count].ip_addr = ip_addr;
    dns_cache[dns_cache_count].ttl = ttl;
    dns_cache[dns_cache_count].expire_time = 0;  // TODO: Get current time + ttl
    
    dns_cache_count++;
    return 0;
}