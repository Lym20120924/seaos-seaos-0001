#ifndef DNS_H
#define DNS_H

#include <stdint.h>
#include <stddef.h>

// DNS header flags
#define DNS_QR_QUERY     0x0000
#define DNS_QR_RESPONSE  0x8000
#define DNS_OPCODE_QUERY 0x0000
#define DNS_AA_FLAG      0x0400
#define DNS_TC_FLAG      0x0200
#define DNS_RD_FLAG      0x0100
#define DNS_RA_FLAG      0x0080
#define DNS_RCODE_MASK   0x000F

// DNS record types
#define DNS_TYPE_A     1   // IPv4 address
#define DNS_TYPE_NS    2   // Nameserver
#define DNS_TYPE_CNAME 5   // Canonical name
#define DNS_TYPE_SOA   6   // Start of authority
#define DNS_TYPE_PTR   12  // Pointer record
#define DNS_TYPE_MX    15  // Mail exchange
#define DNS_TYPE_TXT   16  // Text record
#define DNS_TYPE_AAAA  28  // IPv6 address

// DNS classes
#define DNS_CLASS_IN   1   // Internet

// DNS initialization
void dns_init(void);

// DNS resolver functions
int dns_resolve(const char* hostname, uint32_t* ip_addr);
int dns_set_nameserver(uint32_t nameserver_ip);
void dns_handle_response(const void* packet, size_t length);

// DNS cache management
void dns_cache_clear(void);
int dns_cache_lookup(const char* hostname, uint32_t* ip_addr);
int dns_cache_add(const char* hostname, uint32_t ip_addr, uint32_t ttl);

#endif