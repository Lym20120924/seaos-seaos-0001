#ifndef TCP_H
#define TCP_H

#include <stdint.h>
#include <stddef.h>

// Forward declaration
typedef struct tcp_connection tcp_connection_t;

// TCP connection states
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

// TCP initialization
void tcp_init(void);

// TCP connection management
tcp_connection_t* tcp_connect(uint32_t remote_ip, uint16_t remote_port);
void tcp_disconnect(tcp_connection_t* conn);

// TCP data transfer
int tcp_send(tcp_connection_t* conn, const void* data, size_t length);
int tcp_receive(tcp_connection_t* conn, void* buffer, size_t max_length);

// TCP packet handling
void tcp_handle_packet(const void* packet, size_t length, uint32_t source_ip);

// TCP connection status
int tcp_get_state(tcp_connection_t* conn);
int tcp_is_connected(tcp_connection_t* conn);
size_t tcp_available(tcp_connection_t* conn);

// TCP socket options
int tcp_set_option(tcp_connection_t* conn, int option, const void* value, size_t length);

#endif