#include <stdint.h>

static inline void outb(uint16_t port, uint8_t val) {
    asm volatile ("outb %0, %1" : : "a" (val), "Nd" (port));
}

void main(void) {
    const char *msg = "SeaOS kernel loaded successfully.";
    while (*msg) {
        outb(0xE9, (uint8_t)*msg++); // QEMU/Bochs 控制台端口
    }
    for (;;) {
        asm volatile ("hlt");
    }
}