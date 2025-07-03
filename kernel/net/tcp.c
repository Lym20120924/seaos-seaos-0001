#include <stdint.h>
#include <stddef.h>
#include "../core/kprintf.h"
#include "../core/mm.h"
#include "ip.h"
#include "tcp.h"

#define TCP_MAX_CONNECTIONS 256
#define TCP_BUFFER_SIZE 8192
#define TCP_MSS 1460  // Maximum Segment Size
#define TCP_WINDOW_SIZE 65535
#define TCP_INITIAL_TIMEOUT 1000  // 1 second

// TCP Connection States
#define TCP_CLOSED      0
#define TCP_LISTEN      1
#define TCP_SYN_SENT    2
#define TCP_SYN_RECV    3
#define TCP_ESTABLISHED 4
#define TCP_FIN_WAIT1   5
#define TCP_FIN_WAIT2   6
#define TCP_CLOSING     7
#define TCP_TIME_WAIT   8
#define TCP_CLOSE_WAIT  9
#define TCP_LAST_ACK    10

// TCP Header Flags
#define TCP_FIN  0x01
#define TCP_SYN  0x02
#define TCP_RST  0x04
#define TCP_PSH  0x08
#define TCP_ACK  0x10
#define TCP_URG  0x20

typedef struct {
    uint16_t source_port;
    uint16_t dest_port;
    uint32_t seq_num;
    uint32_t ack_num;
    uint8_t  data_offset;
    uint8_t  flags;
    uint16_t window_size;
    uint16_t checksum;
    uint16_t urgent_ptr;
    uint8_t  options[];
} __attribute__((packed)) tcp_header_t;

typedef struct tcp_connection {
    uint8_t state;
    uint32_t local_ip;
    uint32_t remote_ip;
    uint16_t local_port;
    uint16_t remote_port;
    
    // Sequence numbers
    uint32_t send_seq;
    uint32_t send_ack;
    uint32_t recv_seq;
    uint32_t recv_ack;
    
    // Buffers
    uint8_t* rx_buffer;
    size_t rx_size;
    size_t rx_capacity;
    uint8_t* tx_buffer;
    size_t tx_size;
    size_t tx_capacity;
    
    // Flow control
    uint16_t window_size;
    uint16_t remote_window;
    
    // Timing
    uint32_t retransmit_timeout;
    uint32_t last_activity;
    uint32_t last_ack_time;
    
    // Connection management
    struct tcp_connection* next;
    uint8_t active;
} tcp_connection_t;

static tcp_connection_t connections[TCP_MAX_CONNECTIONS];
static tcp_connection_t* connection_list = NULL;
static uint16_t next_port = 49152; // Dynamic port range start
static uint32_t initial_seq = 0;

void tcp_init(void) {
    kprintf("Initializing TCP stack...\n");
    
    // Initialize connection array
    for (int i = 0; i < TCP_MAX_CONNECTIONS; i++) {
        connections[i].state = TCP_CLOSED;
        connections[i].active = 0;
        connections[i].rx_buffer = NULL;
        connections[i].tx_buffer = NULL;
        connections[i].next = NULL;
    }
    
    connection_list = NULL;
    initial_seq = 1000; // TODO: Use random initial sequence
    
    kprintf("TCP stack initialized\n");
}

static uint16_t tcp_checksum(const tcp_header_t* header, const void* data, size_t length,
                           uint32_t source_ip, uint32_t dest_ip) {
    uint32_t sum = 0;
    
    // Add pseudo-header
    sum += (source_ip >> 16) & 0xFFFF;
    sum += source_ip & 0xFFFF;
    sum += (dest_ip >> 16) & 0xFFFF;
    sum += dest_ip & 0xFFFF;
    sum += htons(6); // Protocol (TCP)
    sum += htons(sizeof(tcp_header_t) + length);
    
    // Add TCP header (excluding checksum field)
    const uint16_t* ptr = (const uint16_t*)header;
    for (size_t i = 0; i < sizeof(tcp_header_t)/2; i++) {
        if (i != 8) { // Skip checksum field
            sum += ptr[i];
        }
    }
    
    // Add data
    ptr = (const uint16_t*)data;
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

static tcp_connection_t* tcp_find_connection(uint32_t remote_ip, uint16_t remote_port, uint16_t local_port) {
    tcp_connection_t* conn = connection_list;
    while (conn) {
        if (conn->active &&
            conn->remote_ip == remote_ip &&
            conn->remote_port == remote_port &&
            conn->local_port == local_port) {
            return conn;
        }
        conn = conn->next;
    }
    return NULL;
}

static tcp_connection_t* tcp_allocate_connection(void) {
    for (int i = 0; i < TCP_MAX_CONNECTIONS; i++) {
        if (!connections[i].active) {
            tcp_connection_t* conn = &connections[i];
            
            // Initialize connection
            memset(conn, 0, sizeof(tcp_connection_t));
            conn->active = 1;
            conn->state = TCP_CLOSED;
            
            // Allocate buffers
            conn->rx_buffer = (uint8_t*)kmalloc(TCP_BUFFER_SIZE);
            conn->tx_buffer = (uint8_t*)kmalloc(TCP_BUFFER_SIZE);
            
            if (!conn->rx_buffer || !conn->tx_buffer) {
                if (conn->rx_buffer) kfree(conn->rx_buffer);
                if (conn->tx_buffer) kfree(conn->tx_buffer);
                conn->active = 0;
                return NULL;
            }
            
            conn->rx_capacity = TCP_BUFFER_SIZE;
            conn->tx_capacity = TCP_BUFFER_SIZE;
            conn->window_size = TCP_WINDOW_SIZE;
            conn->remote_window = TCP_WINDOW_SIZE;
            conn->retransmit_timeout = TCP_INITIAL_TIMEOUT;
            
            // Add to connection list
            conn->next = connection_list;
            connection_list = conn;
            
            return conn;
        }
    }
    return NULL;
}

static void tcp_free_connection(tcp_connection_t* conn) {
    if (!conn || !conn->active) return;
    
    // Remove from connection list
    if (connection_list == conn) {
        connection_list = conn->next;
    } else {
        tcp_connection_t* prev = connection_list;
        while (prev && prev->next != conn) {
            prev = prev->next;
        }
        if (prev) {
            prev->next = conn->next;
        }
    }
    
    // Free buffers
    if (conn->rx_buffer) {
        kfree(conn->rx_buffer);
        conn->rx_buffer = NULL;
    }
    if (conn->tx_buffer) {
        kfree(conn->tx_buffer);
        conn->tx_buffer = NULL;
    }
    
    conn->active = 0;
    conn->state = TCP_CLOSED;
}

static void tcp_send_packet(tcp_connection_t* conn, uint8_t flags, const void* data, size_t length) {
    if (!conn) return;
    
    tcp_header_t header;
    header.source_port = htons(conn->local_port);
    header.dest_port = htons(conn->remote_port);
    header.seq_num = htonl(conn->send_seq);
    header.ack_num = htonl(conn->send_ack);
    header.data_offset = (sizeof(tcp_header_t) / 4) << 4;
    header.flags = flags;
    header.window_size = htons(conn->window_size);
    header.urgent_ptr = 0;
    header.checksum = 0;
    
    // Calculate checksum
    header.checksum = tcp_checksum(&header, data, length, conn->local_ip, conn->remote_ip);
    
    // Allocate packet buffer
    size_t packet_size = sizeof(tcp_header_t) + length;
    uint8_t* packet = (uint8_t*)kmalloc(packet_size);
    if (!packet) return;
    
    // Build packet
    memcpy(packet, &header, sizeof(tcp_header_t));
    if (data && length > 0) {
        memcpy(packet + sizeof(tcp_header_t), data, length);
    }
    
    // Send via IP
    ip_send_packet(conn->remote_ip, 6, packet, packet_size);
    
    // Update sequence number for data packets
    if (length > 0) {
        conn->send_seq += length;
    }
    
    // Update sequence number for SYN/FIN
    if (flags & (TCP_SYN | TCP_FIN)) {
        conn->send_seq++;
    }
    
    kfree(packet);
}

tcp_connection_t* tcp_connect(uint32_t remote_ip, uint16_t remote_port) {
    tcp_connection_t* conn = tcp_allocate_connection();
    if (!conn) return NULL;
    
    conn->state = TCP_SYN_SENT;
    conn->local_ip = ip_get_local_address();
    conn->remote_ip = remote_ip;
    conn->local_port = next_port++;
    conn->remote_port = remote_port;
    conn->send_seq = initial_seq++;
    conn->send_ack = 0;
    
    // Send SYN packet
    tcp_send_packet(conn, TCP_SYN, NULL, 0);
    
    kprintf("TCP: Connecting to %d.%d.%d.%d:%d\n",
            (remote_ip >> 24) & 0xFF, (remote_ip >> 16) & 0xFF,
            (remote_ip >> 8) & 0xFF, remote_ip & 0xFF, remote_port);
    
    return conn;
}

void tcp_disconnect(tcp_connection_t* conn) {
    if (!conn || conn->state == TCP_CLOSED) return;
    
    switch (conn->state) {
        case TCP_ESTABLISHED:
        case TCP_CLOSE_WAIT:
            // Send FIN packet
            tcp_send_packet(conn, TCP_FIN | TCP_ACK, NULL, 0);
            conn->state = (conn->state == TCP_ESTABLISHED) ? TCP_FIN_WAIT1 : TCP_LAST_ACK;
            break;
            
        default:
            // Force close
            tcp_free_connection(conn);
            break;
    }
}

int tcp_send(tcp_connection_t* conn, const void* data, size_t length) {
    if (!conn || conn->state != TCP_ESTABLISHED || !data || length == 0) {
        return -1;
    }
    
    // Check remote window
    if (length > conn->remote_window) {
        return -1; // Would exceed remote window
    }
    
    // Fragment large data into MSS-sized segments
    size_t sent = 0;
    const uint8_t* ptr = (const uint8_t*)data;
    
    while (sent < length) {
        size_t segment_size = (length - sent > TCP_MSS) ? TCP_MSS : (length - sent);
        
        // Send segment
        tcp_send_packet(conn, TCP_PSH | TCP_ACK, ptr + sent, segment_size);
        
        sent += segment_size;
        conn->remote_window -= segment_size;
    }
    
    return sent;
}

int tcp_receive(tcp_connection_t* conn, void* buffer, size_t max_length) {
    if (!conn || conn->state != TCP_ESTABLISHED || !buffer || max_length == 0) {
        return -1;
    }
    
    // Check if there's data available
    if (conn->rx_size == 0) return 0;
    
    // Copy data to user buffer
    size_t copy_size = (max_length < conn->rx_size) ? max_length : conn->rx_size;
    memcpy(buffer, conn->rx_buffer, copy_size);
    
    // Move remaining data to start of buffer
    if (copy_size < conn->rx_size) {
        memmove(conn->rx_buffer, conn->rx_buffer + copy_size, conn->rx_size - copy_size);
    }
    
    conn->rx_size -= copy_size;
    
    // Update window size
    conn->window_size += copy_size;
    
    return copy_size;
}

void tcp_handle_packet(const void* packet, size_t length, uint32_t source_ip) {
    if (!packet || length < sizeof(tcp_header_t)) return;
    
    const tcp_header_t* header = (const tcp_header_t*)packet;
    const void* data = (const uint8_t*)packet + sizeof(tcp_header_t);
    size_t data_length = length - sizeof(tcp_header_t);
    
    uint16_t source_port = ntohs(header->source_port);
    uint16_t dest_port = ntohs(header->dest_port);
    uint32_t seq_num = ntohl(header->seq_num);
    uint32_t ack_num = ntohl(header->ack_num);
    uint16_t window = ntohs(header->window_size);
    
    // Find matching connection
    tcp_connection_t* conn = tcp_find_connection(source_ip, source_port, dest_port);
    
    // Handle new connections (SYN packets to listening ports)
    if (!conn && (header->flags & TCP_SYN) && !(header->flags & TCP_ACK)) {
        // TODO: Implement listening sockets
        return;
    }
    
    if (!conn) return;
    
    // Verify checksum
    uint16_t orig_checksum = header->checksum;
    ((tcp_header_t*)header)->checksum = 0;
    if (tcp_checksum(header, data, data_length, source_ip, conn->local_ip) != orig_checksum) {
        return;
    }
    
    // Update connection state
    conn->last_activity = 0; // TODO: Get current time
    conn->remote_window = window;
    
    // Handle packet based on connection state
    switch (conn->state) {
        case TCP_SYN_SENT:
            if ((header->flags & (TCP_SYN | TCP_ACK)) == (TCP_SYN | TCP_ACK)) {
                if (ack_num == conn->send_seq) {
                    conn->state = TCP_ESTABLISHED;
                    conn->send_ack = seq_num + 1;
                    conn->recv_seq = seq_num + 1;
                    
                    // Send ACK
                    tcp_send_packet(conn, TCP_ACK, NULL, 0);
                    
                    kprintf("TCP: Connection established\n");
                }
            } else if (header->flags & TCP_RST) {
                tcp_free_connection(conn);
            }
            break;
            
        case TCP_ESTABLISHED:
            // Handle data
            if (data_length > 0 && seq_num == conn->recv_seq) {
                if (conn->rx_size + data_length <= conn->rx_capacity) {
                    memcpy(conn->rx_buffer + conn->rx_size, data, data_length);
                    conn->rx_size += data_length;
                    conn->recv_seq += data_length;
                    conn->send_ack = conn->recv_seq;
                    conn->window_size -= data_length;
                    
                    // Send ACK
                    tcp_send_packet(conn, TCP_ACK, NULL, 0);
                }
            }
            
            // Handle ACK
            if (header->flags & TCP_ACK) {
                // Update send window based on ACK
                if (ack_num > conn->recv_ack) {
                    conn->recv_ack = ack_num;
                }
            }
            
            // Handle FIN
            if (header->flags & TCP_FIN) {
                conn->state = TCP_CLOSE_WAIT;
                conn->recv_seq++;
                conn->send_ack = conn->recv_seq;
                
                // Send ACK
                tcp_send_packet(conn, TCP_ACK, NULL, 0);
            }
            
            // Handle RST
            if (header->flags & TCP_RST) {
                tcp_free_connection(conn);
            }
            break;
            
        case TCP_FIN_WAIT1:
            if (header->flags & TCP_ACK && ack_num == conn->send_seq) {
                conn->state = TCP_FIN_WAIT2;
            }
            if (header->flags & TCP_FIN) {
                conn->recv_seq++;
                conn->send_ack = conn->recv_seq;
                tcp_send_packet(conn, TCP_ACK, NULL, 0);
                
                if (conn->state == TCP_FIN_WAIT2) {
                    conn->state = TCP_TIME_WAIT;
                    // TODO: Start TIME_WAIT timer
                } else {
                    conn->state = TCP_CLOSING;
                }
            }
            break;
            
        case TCP_FIN_WAIT2:
            if (header->flags & TCP_FIN) {
                conn->recv_seq++;
                conn->send_ack = conn->recv_seq;
                tcp_send_packet(conn, TCP_ACK, NULL, 0);
                conn->state = TCP_TIME_WAIT;
                // TODO: Start TIME_WAIT timer
            }
            break;
            
        case TCP_CLOSE_WAIT:
            // Application should close the connection
            break;
            
        case TCP_LAST_ACK:
            if (header->flags & TCP_ACK && ack_num == conn->send_seq) {
                tcp_free_connection(conn);
            }
            break;
            
        case TCP_CLOSING:
            if (header->flags & TCP_ACK && ack_num == conn->send_seq) {
                conn->state = TCP_TIME_WAIT;
                // TODO: Start TIME_WAIT timer
            }
            break;
            
        case TCP_TIME_WAIT:
            // TODO: Handle TIME_WAIT timeout
            break;
            
        default:
            break;
    }
}

// Get connection state
int tcp_get_state(tcp_connection_t* conn) {
    return conn ? conn->state : TCP_CLOSED;
}

// Check if connection is established
int tcp_is_connected(tcp_connection_t* conn) {
    return conn && conn->state == TCP_ESTABLISHED;
}

// Get available data size
size_t tcp_available(tcp_connection_t* conn) {
    return conn ? conn->rx_size : 0;
}

// Set socket options
int tcp_set_option(tcp_connection_t* conn, int option, const void* value, size_t length) {
    if (!conn || !value) return -1;
    
    switch (option) {
        case 1: // SO_REUSEADDR
            // TODO: Implement socket options
            break;
        case 2: // SO_KEEPALIVE
            // TODO: Implement keepalive
            break;
        default:
            return -1;
    }
    
    return 0;
}