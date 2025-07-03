#include "kprintf.h"

void kernel_panic(const char* message) {
    // Disable interrupts
    __asm__ volatile("cli");
    
    // Print panic message
    kprintf("\nKERNEL PANIC: %s\n", message);
    
    // Halt the system
    while(1) {
        __asm__ volatile("hlt");
    }
}