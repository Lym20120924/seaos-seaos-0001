#include <stdint.h>

#define VGA_TEXT_BUF ((volatile uint16_t*)0xB8000)

static void vga_print(const char *s, uint8_t color) {
    static uint16_t *pos = (uint16_t*)VGA_TEXT_BUF;
    while (*s) {
        if (*s == '\n') {
            uint32_t offset = (uint32_t)(pos - VGA_TEXT_BUF);
            offset = (offset / 80 + 1) * 80; // next line
            pos = (uint16_t*)VGA_TEXT_BUF + offset;
            ++s;
            continue;
        }
        *pos++ = ((uint16_t)color << 8) | *s++;
    }
}

/* ------------------ Simple Paging ------------------ */
#define PAGE_PRESENT 0x1
#define PAGE_RW      0x2

static uint32_t __attribute__((aligned(4096))) page_table[1024];
static uint32_t __attribute__((aligned(4096))) page_directory[1024];

static void paging_init(void) {
    /* Identity-map first 4 MiB  */
    for (uint32_t i = 0; i < 1024; ++i) {
        page_table[i] = (i * 0x1000) | PAGE_PRESENT | PAGE_RW;
        page_directory[i] = 0;            // init
    }

    page_directory[0] = ((uint32_t)page_table) | PAGE_PRESENT | PAGE_RW;

    /* Load page directory base */
    asm volatile ("mov %0, %%cr3" :: "r"(page_directory));

    /* Enable paging bit in CR0 */
    uint32_t cr0;
    asm volatile ("mov %%cr0, %0" : "=r"(cr0));
    cr0 |= 0x80000000; // PG
    asm volatile ("mov %0, %%cr0" :: "r"(cr0));
}

static inline void outb(uint16_t port, uint8_t val) {
    asm volatile ("outb %0, %1" : : "a" (val), "Nd" (port));
}

void _start(void) {
    const char *msg = "SeaOS kernel: Protected mode & paging ON.";
    while (*msg) {
        outb(0xE9, (uint8_t)*msg++);
    }
    /* Init paging */
    paging_init();

    /* Print to VGA */
    vga_print("SeaOS boot successful!\n", 0x0F);

    for (;;) {
        asm volatile ("hlt");
    }
}