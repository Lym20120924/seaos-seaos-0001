#ifndef KPRINTF_H
#define KPRINTF_H

#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>

// Core printf functions
void kprintf_init(void);
void kprintf(const char* format, ...);
void set_terminal_color(uint8_t color);

// String manipulation functions
size_t strlen(const char* str);
void* memset(void* ptr, int value, size_t num);
void* memcpy(void* dest, const void* src, size_t num);
void* memmove(void* dest, const void* src, size_t num);
int memcmp(const void* ptr1, const void* ptr2, size_t num);
char* strcpy(char* dest, const char* src);
char* strncpy(char* dest, const char* src, size_t num);
int strcmp(const char* str1, const char* str2);
int strncmp(const char* str1, const char* str2, size_t num);

// Network byte order functions
uint16_t htons(uint16_t hostshort);
uint16_t ntohs(uint16_t netshort);
uint32_t htonl(uint32_t hostlong);
uint32_t ntohl(uint32_t netlong);

// I/O port functions
void outb(uint16_t port, uint8_t value);
uint8_t inb(uint16_t port);
void outw(uint16_t port, uint16_t value);
uint16_t inw(uint16_t port);
void outl(uint16_t port, uint32_t value);
uint32_t inl(uint16_t port);

// Printf variants
int snprintf(char* buffer, size_t size, const char* format, ...);
int sscanf(const char* str, const char* format, ...);

// Terminal colors
#define TERMINAL_COLOR_BLACK         0
#define TERMINAL_COLOR_BLUE          1
#define TERMINAL_COLOR_GREEN         2
#define TERMINAL_COLOR_CYAN          3
#define TERMINAL_COLOR_RED           4
#define TERMINAL_COLOR_MAGENTA       5
#define TERMINAL_COLOR_BROWN         6
#define TERMINAL_COLOR_LIGHT_GREY    7
#define TERMINAL_COLOR_DARK_GREY     8
#define TERMINAL_COLOR_LIGHT_BLUE    9
#define TERMINAL_COLOR_LIGHT_GREEN   10
#define TERMINAL_COLOR_LIGHT_CYAN    11
#define TERMINAL_COLOR_LIGHT_RED     12
#define TERMINAL_COLOR_LIGHT_MAGENTA 13
#define TERMINAL_COLOR_LIGHT_BROWN   14
#define TERMINAL_COLOR_WHITE         15

#define MAKE_COLOR(fg, bg) ((bg << 4) | (fg & 0x0F))

#endif