#include <stdarg.h>
#include <stddef.h>

#define VGA_WIDTH 80
#define VGA_HEIGHT 25

static uint16_t* const VGA_MEMORY = (uint16_t*)0xB8000;
static size_t terminal_row = 0;
static size_t terminal_column = 0;
static uint8_t terminal_color = 0x07; // Light gray on black

// String manipulation functions
size_t strlen(const char* str) {
    size_t len = 0;
    while (str[len]) len++;
    return len;
}

void* memset(void* ptr, int value, size_t num) {
    unsigned char* p = (unsigned char*)ptr;
    while (num--) *p++ = (unsigned char)value;
    return ptr;
}

void* memcpy(void* dest, const void* src, size_t num) {
    unsigned char* d = (unsigned char*)dest;
    const unsigned char* s = (const unsigned char*)src;
    while (num--) *d++ = *s++;
    return dest;
}

void* memmove(void* dest, const void* src, size_t num) {
    unsigned char* d = (unsigned char*)dest;
    const unsigned char* s = (const unsigned char*)src;
    
    if (d < s) {
        while (num--) *d++ = *s++;
    } else {
        d += num;
        s += num;
        while (num--) *--d = *--s;
    }
    return dest;
}

int memcmp(const void* ptr1, const void* ptr2, size_t num) {
    const unsigned char* p1 = (const unsigned char*)ptr1;
    const unsigned char* p2 = (const unsigned char*)ptr2;
    
    while (num--) {
        if (*p1 != *p2) {
            return (*p1 < *p2) ? -1 : 1;
        }
        p1++;
        p2++;
    }
    return 0;
}

char* strcpy(char* dest, const char* src) {
    char* d = dest;
    while ((*d++ = *src++));
    return dest;
}

char* strncpy(char* dest, const char* src, size_t num) {
    char* d = dest;
    while (num-- && (*d++ = *src++));
    while (num--) *d++ = '\0';
    return dest;
}

int strcmp(const char* str1, const char* str2) {
    while (*str1 && (*str1 == *str2)) {
        str1++;
        str2++;
    }
    return *(unsigned char*)str1 - *(unsigned char*)str2;
}

int strncmp(const char* str1, const char* str2, size_t num) {
    while (num-- && *str1 && (*str1 == *str2)) {
        str1++;
        str2++;
    }
    return num ? *(unsigned char*)str1 - *(unsigned char*)str2 : 0;
}

// Network byte order functions
uint16_t htons(uint16_t hostshort) {
    return ((hostshort & 0xFF) << 8) | ((hostshort >> 8) & 0xFF);
}

uint16_t ntohs(uint16_t netshort) {
    return htons(netshort); // Same operation
}

uint32_t htonl(uint32_t hostlong) {
    return ((hostlong & 0xFF) << 24) |
           (((hostlong >> 8) & 0xFF) << 16) |
           (((hostlong >> 16) & 0xFF) << 8) |
           ((hostlong >> 24) & 0xFF);
}

uint32_t ntohl(uint32_t netlong) {
    return htonl(netlong); // Same operation
}

// I/O port functions
void outb(uint16_t port, uint8_t value) {
    __asm__ volatile("outb %0, %1" : : "a"(value), "Nd"(port));
}

uint8_t inb(uint16_t port) {
    uint8_t value;
    __asm__ volatile("inb %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

void outw(uint16_t port, uint16_t value) {
    __asm__ volatile("outw %0, %1" : : "a"(value), "Nd"(port));
}

uint16_t inw(uint16_t port) {
    uint16_t value;
    __asm__ volatile("inw %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

void outl(uint16_t port, uint32_t value) {
    __asm__ volatile("outl %0, %1" : : "a"(value), "Nd"(port));
}

uint32_t inl(uint16_t port) {
    uint32_t value;
    __asm__ volatile("inl %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

// Printf implementation
static void print_char(char c) {
    if (c == '\n') {
        terminal_column = 0;
        terminal_row++;
        if (terminal_row >= VGA_HEIGHT) {
            // Scroll screen
            for (size_t y = 0; y < VGA_HEIGHT - 1; y++) {
                for (size_t x = 0; x < VGA_WIDTH; x++) {
                    VGA_MEMORY[y * VGA_WIDTH + x] = VGA_MEMORY[(y + 1) * VGA_WIDTH + x];
                }
            }
            // Clear last line
            for (size_t x = 0; x < VGA_WIDTH; x++) {
                VGA_MEMORY[(VGA_HEIGHT - 1) * VGA_WIDTH + x] = (uint16_t)' ' | (uint16_t)(terminal_color << 8);
            }
            terminal_row = VGA_HEIGHT - 1;
        }
        return;
    }
    
    if (terminal_column >= VGA_WIDTH) {
        terminal_column = 0;
        terminal_row++;
        if (terminal_row >= VGA_HEIGHT) {
            terminal_row = VGA_HEIGHT - 1;
        }
    }
    
    const size_t index = terminal_row * VGA_WIDTH + terminal_column;
    VGA_MEMORY[index] = (uint16_t)c | (uint16_t)(terminal_color << 8);
    terminal_column++;
}

static void print_string(const char* str) {
    while (*str) {
        print_char(*str++);
    }
}

static void print_number(uint64_t num, int base, int uppercase) {
    char buffer[32];
    int i = 0;
    
    if (num == 0) {
        print_char('0');
        return;
    }
    
    const char* digits = uppercase ? "0123456789ABCDEF" : "0123456789abcdef";
    
    while (num > 0) {
        buffer[i++] = digits[num % base];
        num /= base;
    }
    
    while (--i >= 0) {
        print_char(buffer[i]);
    }
}

void kprintf_init(void) {
    // Clear the screen
    for (size_t y = 0; y < VGA_HEIGHT; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            const size_t index = y * VGA_WIDTH + x;
            VGA_MEMORY[index] = (uint16_t)' ' | (uint16_t)(terminal_color << 8);
        }
    }
    terminal_row = 0;
    terminal_column = 0;
}

void kprintf(const char* format, ...) {
    va_list args;
    va_start(args, format);
    
    while (*format) {
        if (*format == '%') {
            format++;
            switch (*format) {
                case 's': {
                    char* str = va_arg(args, char*);
                    print_string(str ? str : "(null)");
                    break;
                }
                case 'd': {
                    int num = va_arg(args, int);
                    if (num < 0) {
                        print_char('-');
                        num = -num;
                    }
                    print_number(num, 10, 0);
                    break;
                }
                case 'u': {
                    unsigned int num = va_arg(args, unsigned int);
                    print_number(num, 10, 0);
                    break;
                }
                case 'x': {
                    unsigned int num = va_arg(args, unsigned int);
                    print_number(num, 16, 0);
                    break;
                }
                case 'X': {
                    unsigned int num = va_arg(args, unsigned int);
                    print_number(num, 16, 1);
                    break;
                }
                case 'p': {
                    void* ptr = va_arg(args, void*);
                    print_string("0x");
                    print_number((uintptr_t)ptr, 16, 0);
                    break;
                }
                case 'c': {
                    char c = (char)va_arg(args, int);
                    print_char(c);
                    break;
                }
                case '%': {
                    print_char('%');
                    break;
                }
                default: {
                    print_char('%');
                    print_char(*format);
                    break;
                }
            }
        } else {
            print_char(*format);
        }
        format++;
    }
    
    va_end(args);
}

void set_terminal_color(uint8_t color) {
    terminal_color = color;
}

// Simple sprintf implementation
int snprintf(char* buffer, size_t size, const char* format, ...) {
    va_list args;
    va_start(args, format);
    
    size_t pos = 0;
    
    while (*format && pos < size - 1) {
        if (*format == '%') {
            format++;
            switch (*format) {
                case 'd': {
                    int num = va_arg(args, int);
                    char temp[16];
                    int len = 0;
                    
                    if (num < 0) {
                        if (pos < size - 1) buffer[pos++] = '-';
                        num = -num;
                    }
                    
                    if (num == 0) {
                        temp[len++] = '0';
                    } else {
                        while (num > 0) {
                            temp[len++] = '0' + (num % 10);
                            num /= 10;
                        }
                    }
                    
                    while (len > 0 && pos < size - 1) {
                        buffer[pos++] = temp[--len];
                    }
                    break;
                }
                case 'u': {
                    unsigned int num = va_arg(args, unsigned int);
                    char temp[16];
                    int len = 0;
                    
                    if (num == 0) {
                        temp[len++] = '0';
                    } else {
                        while (num > 0) {
                            temp[len++] = '0' + (num % 10);
                            num /= 10;
                        }
                    }
                    
                    while (len > 0 && pos < size - 1) {
                        buffer[pos++] = temp[--len];
                    }
                    break;
                }
                case 's': {
                    char* str = va_arg(args, char*);
                    if (!str) str = "(null)";
                    while (*str && pos < size - 1) {
                        buffer[pos++] = *str++;
                    }
                    break;
                }
                default:
                    if (pos < size - 1) buffer[pos++] = *format;
                    break;
            }
        } else {
            buffer[pos++] = *format;
        }
        format++;
    }
    
    buffer[pos] = '\0';
    va_end(args);
    return pos;
}

// Simple sscanf implementation
int sscanf(const char* str, const char* format, ...) {
    va_list args;
    va_start(args, format);
    
    int count = 0;
    
    while (*format && *str) {
        if (*format == '%') {
            format++;
            switch (*format) {
                case 'u': {
                    unsigned int* num = va_arg(args, unsigned int*);
                    *num = 0;
                    
                    while (*str >= '0' && *str <= '9') {
                        *num = *num * 10 + (*str - '0');
                        str++;
                    }
                    count++;
                    break;
                }
                case 'd': {
                    int* num = va_arg(args, int*);
                    int sign = 1;
                    
                    if (*str == '-') {
                        sign = -1;
                        str++;
                    }
                    
                    *num = 0;
                    while (*str >= '0' && *str <= '9') {
                        *num = *num * 10 + (*str - '0');
                        str++;
                    }
                    *num *= sign;
                    count++;
                    break;
                }
                default:
                    break;
            }
        } else if (*format == *str) {
            str++;
        } else {
            break;
        }
        format++;
    }
    
    va_end(args);
    return count;
}