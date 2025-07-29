#include <stdint.h>

static inline void outb(uint16_t port, uint8_t val) {
    asm volatile ("outb %0, %1" : : "a" (val), "Nd" (port));
}

void _start(void) {
    const char *msg = "SeaOS 32-bit kernel started.";
    while (*msg) {
        outb(0xE9, (uint8_t)*msg++);
    }
    for (;;) {
        asm volatile ("hlt");
    }
}