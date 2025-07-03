#include <stdint.h>
#include "../../core/kprintf.h"

#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define VGA_MEMORY 0xB8000

typedef struct {
    uint8_t character;
    uint8_t color;
} vga_char;

static vga_char* vga_buffer = (vga_char*)VGA_MEMORY;
static uint8_t vga_color = 0x07; // Light gray on black
static uint8_t cursor_x = 0;
static uint8_t cursor_y = 0;

void vga_init(void) {
    kprintf("Initializing VGA driver...\n");
    
    // Clear screen
    for (int y = 0; y < VGA_HEIGHT; y++) {
        for (int x = 0; x < VGA_WIDTH; x++) {
            vga_buffer[y * VGA_WIDTH + x].character = ' ';
            vga_buffer[y * VGA_WIDTH + x].color = vga_color;
        }
    }
    
    kprintf("VGA driver initialized\n");
}

void vga_putchar(char c) {
    if (c == '\n') {
        cursor_x = 0;
        cursor_y++;
        if (cursor_y >= VGA_HEIGHT) {
            // Scroll screen
            for (int y = 0; y < VGA_HEIGHT - 1; y++) {
                for (int x = 0; x < VGA_WIDTH; x++) {
                    vga_buffer[y * VGA_WIDTH + x] = vga_buffer[(y + 1) * VGA_WIDTH + x];
                }
            }
            cursor_y = VGA_HEIGHT - 1;
        }
        return;
    }
    
    vga_buffer[cursor_y * VGA_WIDTH + cursor_x].character = c;
    vga_buffer[cursor_y * VGA_WIDTH + cursor_x].color = vga_color;
    
    cursor_x++;
    if (cursor_x >= VGA_WIDTH) {
        cursor_x = 0;
        cursor_y++;
        if (cursor_y >= VGA_HEIGHT) {
            // Scroll screen
            for (int y = 0; y < VGA_HEIGHT - 1; y++) {
                for (int x = 0; x < VGA_WIDTH; x++) {
                    vga_buffer[y * VGA_WIDTH + x] = vga_buffer[(y + 1) * VGA_WIDTH + x];
                }
            }
            cursor_y = VGA_HEIGHT - 1;
        }
    }
}