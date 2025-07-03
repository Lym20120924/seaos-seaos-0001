#include <stdint.h>
#include "../../core/kprintf.h"

#define PS2_DATA_PORT    0x60
#define PS2_STATUS_PORT  0x64
#define PS2_COMMAND_PORT 0x64

#define PS2_ACK         0xFA
#define PS2_RESEND      0xFE

// PS/2 keyboard scan code set 1
#define KEY_ESCAPE      0x01
#define KEY_BACKSPACE   0x0E
#define KEY_TAB         0x0F
#define KEY_ENTER       0x1C
#define KEY_CTRL        0x1D
#define KEY_SHIFT       0x2A
#define KEY_SPACE       0x39

typedef struct {
    uint8_t scancode;
    uint8_t ascii;
    uint8_t shift_ascii;
    uint8_t flags;
} keymap_entry_t;

static uint8_t keyboard_state = 0;
static uint8_t mouse_state = 0;

void ps2_init(void) {
    kprintf("Initializing PS/2 controller...\n");
    
    // Disable devices
    outb(PS2_COMMAND_PORT, 0xAD);  // Disable keyboard
    outb(PS2_COMMAND_PORT, 0xA7);  // Disable mouse
    
    // Flush output buffer
    while ((inb(PS2_STATUS_PORT) & 1) == 1) {
        inb(PS2_DATA_PORT);
    }
    
    // Configure controller
    outb(PS2_COMMAND_PORT, 0x20);  // Read configuration
    uint8_t config = inb(PS2_DATA_PORT);
    config |= 0x03;  // Enable interrupts for both ports
    outb(PS2_COMMAND_PORT, 0x60);  // Write configuration
    outb(PS2_DATA_PORT, config);
    
    // Enable devices
    outb(PS2_COMMAND_PORT, 0xAE);  // Enable keyboard
    outb(PS2_COMMAND_PORT, 0xA8);  // Enable mouse
    
    // Reset devices
    ps2_keyboard_reset();
    ps2_mouse_reset();
    
    kprintf("PS/2 controller initialized\n");
}

void ps2_keyboard_reset(void) {
    outb(PS2_DATA_PORT, 0xFF);
    while (inb(PS2_DATA_PORT) != PS2_ACK);
    
    // Set scan code set 1
    outb(PS2_DATA_PORT, 0xF0);
    while (inb(PS2_DATA_PORT) != PS2_ACK);
    outb(PS2_DATA_PORT, 0x01);
    while (inb(PS2_DATA_PORT) != PS2_ACK);
}

void ps2_mouse_reset(void) {
    outb(PS2_COMMAND_PORT, 0xD4);
    outb(PS2_DATA_PORT, 0xFF);
    while (inb(PS2_DATA_PORT) != PS2_ACK);
    
    // Enable data reporting
    outb(PS2_COMMAND_PORT, 0xD4);
    outb(PS2_DATA_PORT, 0xF4);
    while (inb(PS2_DATA_PORT) != PS2_ACK);
}

void ps2_keyboard_handler(void) {
    uint8_t scancode = inb(PS2_DATA_PORT);
    
    // Handle keyboard input
    if (scancode & 0x80) {
        // Key release
        scancode &= 0x7F;
        // TODO: Update key state
    } else {
        // Key press
        // TODO: Process keypress
        kprintf("Key pressed: %02x\n", scancode);
    }
}

void ps2_mouse_handler(void) {
    static uint8_t mouse_cycle = 0;
    static uint8_t mouse_byte[3];
    
    mouse_byte[mouse_cycle++] = inb(PS2_DATA_PORT);
    
    if (mouse_cycle == 3) {
        mouse_cycle = 0;
        
        int8_t mouse_dx = mouse_byte[1];
        int8_t mouse_dy = mouse_byte[2];
        uint8_t mouse_buttons = mouse_byte[0];
        
        // TODO: Process mouse movement and buttons
        kprintf("Mouse: buttons=%02x, dx=%d, dy=%d\n", 
                mouse_buttons, mouse_dx, mouse_dy);
    }
}