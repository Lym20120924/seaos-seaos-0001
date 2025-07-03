#include <stddef.h>
#include <stdint.h>
#include "kprintf.h"
#include "syscall.h"
#include "panic.h"
#include "mm.h"
#include "process.h"
#include "../include/platform.h"

// Kernel entry point
void kernel_main(void) {
    // Initialize kernel logging
    kprintf_init();
    set_terminal_color(MAKE_COLOR(TERMINAL_COLOR_LIGHT_GREEN, TERMINAL_COLOR_BLACK));
    kprintf("SeaOS Kernel Starting...\n");
    set_terminal_color(MAKE_COLOR(TERMINAL_COLOR_LIGHT_GREY, TERMINAL_COLOR_BLACK));

    // Initialize memory management
    kprintf("Initializing memory management...\n");
    mm_init();
    
    // Initialize process manager
    kprintf("Initializing process manager...\n");
    process_init();

    // Initialize system call table
    kprintf("Initializing system calls...\n");
    syscall_init();
    
    // Detect and initialize platform
    kprintf("Detecting platform...\n");
    int platform_type = platform_detect();
    kprintf("Platform detected: %s\n", platform_get_name());
    
    // Platform-specific initialization
    platform_init();
    
    // Display platform capabilities
    platform_capabilities_t caps;
    platform_get_capabilities(&caps);
    
    kprintf("\nPlatform Capabilities:\n");
    kprintf("  Keyboard: %s\n", caps.has_keyboard ? "Yes" : "No");
    kprintf("  Mouse: %s\n", caps.has_mouse ? "Yes" : "No");
    kprintf("  Touchscreen: %s\n", caps.has_touchscreen ? "Yes" : "No");
    kprintf("  WiFi: %s\n", caps.has_wifi ? "Yes" : "No");
    kprintf("  Bluetooth: %s\n", caps.has_bluetooth ? "Yes" : "No");
    kprintf("  Cellular: %s\n", caps.has_cellular ? "Yes" : "No");
    kprintf("  GPS: %s\n", caps.has_gps ? "Yes" : "No");
    kprintf("  Sensors: %s\n", caps.has_sensors ? "Yes" : "No");
    kprintf("  Camera: %s\n", caps.has_camera ? "Yes" : "No");
    kprintf("  Battery: %s\n", caps.has_battery ? "Yes" : "No");
    kprintf("  Mobile: %s\n", caps.is_mobile ? "Yes" : "No");
    kprintf("  Rotation: %s\n", caps.supports_rotation ? "Yes" : "No");

    // Create initial kernel processes
    kprintf("\nCreating initial processes...\n");
    create_process("init");
    
    set_terminal_color(MAKE_COLOR(TERMINAL_COLOR_LIGHT_GREEN, TERMINAL_COLOR_BLACK));
    kprintf("\nSeaOS Kernel initialization complete!\n");
    kprintf("Platform: %s\n", platform_get_name());
    kprintf("Ready for operation.\n");
    set_terminal_color(MAKE_COLOR(TERMINAL_COLOR_LIGHT_GREY, TERMINAL_COLOR_BLACK));
    
    // Start scheduler
    kprintf("\nStarting scheduler...\n");
    schedule();
}

// Platform detection implementation
int platform_detect(void) {
    // Simple platform detection logic
    // In a real implementation, this would check hardware signatures,
    // ACPI tables, device tree, etc.
    
    #ifdef PLATFORM_PC_BUILD
    return PLATFORM_PC;
    #elif defined(PLATFORM_PE_BUILD)
    return PLATFORM_PE;
    #elif defined(PLATFORM_TABLET_BUILD)
    return PLATFORM_TABLET;
    #else
    // Default to PC platform
    return PLATFORM_PC;
    #endif
}

const char* platform_get_name(void) {
    int platform = platform_detect();
    switch (platform) {
        case PLATFORM_PC:
            return "PC Desktop/Laptop";
        case PLATFORM_PE:
            return "Windows PE Environment";
        case PLATFORM_TABLET:
            return "Tablet Device";
        default:
            return "Unknown Platform";
    }
}

void platform_init(void) {
    int platform = platform_detect();
    switch (platform) {
        case PLATFORM_PC:
            pc_platform_init();
            break;
        case PLATFORM_PE:
            pe_platform_init();
            break;
        case PLATFORM_TABLET:
            tablet_platform_init();
            break;
        default:
            kprintf("Warning: Unknown platform, using PC defaults\n");
            pc_platform_init();
            break;
    }
}

void platform_get_capabilities(platform_capabilities_t* caps) {
    if (!caps) return;
    
    memset(caps, 0, sizeof(platform_capabilities_t));
    
    int platform = platform_detect();
    switch (platform) {
        case PLATFORM_PC:
            caps->has_keyboard = 1;
            caps->has_mouse = 1;
            caps->has_touchscreen = 0;
            caps->has_wifi = 1;
            caps->has_bluetooth = 1;
            caps->has_cellular = 0;
            caps->has_gps = 0;
            caps->has_sensors = 0;
            caps->has_camera = 1;
            caps->has_battery = 1;  // Laptops have batteries
            caps->is_mobile = 0;
            caps->supports_rotation = 0;
            break;
            
        case PLATFORM_PE:
            caps->has_keyboard = 1;
            caps->has_mouse = 1;
            caps->has_touchscreen = 0;
            caps->has_wifi = 1;
            caps->has_bluetooth = 0;
            caps->has_cellular = 0;
            caps->has_gps = 0;
            caps->has_sensors = 0;
            caps->has_camera = 0;
            caps->has_battery = 0;
            caps->is_mobile = 0;
            caps->supports_rotation = 0;
            break;
            
        case PLATFORM_TABLET:
            caps->has_keyboard = 0;  // Virtual keyboard
            caps->has_mouse = 0;
            caps->has_touchscreen = 1;
            caps->has_wifi = 1;
            caps->has_bluetooth = 1;
            caps->has_cellular = 1;
            caps->has_gps = 1;
            caps->has_sensors = 1;
            caps->has_camera = 1;
            caps->has_battery = 1;
            caps->is_mobile = 1;
            caps->supports_rotation = 1;
            break;
    }
}

void platform_shutdown(void) {
    kprintf("Shutting down platform...\n");
    
    int platform = platform_detect();
    switch (platform) {
        case PLATFORM_PC:
            // Use ACPI shutdown
            // acpi_shutdown();
            break;
        case PLATFORM_PE:
            // PE environment shutdown
            break;
        case PLATFORM_TABLET:
            // Tablet power off
            // tablet_power_off();
            break;
    }
    
    // Fallback: halt CPU
    __asm__ volatile("cli; hlt");
}

void platform_reboot(void) {
    kprintf("Rebooting platform...\n");
    
    int platform = platform_detect();
    switch (platform) {
        case PLATFORM_PC:
            // Use ACPI reboot
            // acpi_reboot();
            break;
        case PLATFORM_PE:
            // PE environment reboot
            break;
        case PLATFORM_TABLET:
            // Tablet restart
            // tablet_restart();
            break;
    }
    
    // Fallback: keyboard controller reset
    outb(0x64, 0xFE);
    
    // If that fails, triple fault
    __asm__ volatile("cli; hlt");
}