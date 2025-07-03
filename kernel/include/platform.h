#ifndef PLATFORM_H
#define PLATFORM_H

// Platform types
#define PLATFORM_PC     1
#define PLATFORM_PE     2
#define PLATFORM_TABLET 3

// Platform detection
int platform_detect(void);
const char* platform_get_name(void);

// Platform-specific initialization
void pc_platform_init(void);
void pe_platform_init(void);
void tablet_platform_init(void);

// Common platform interface
void platform_init(void);
void platform_shutdown(void);
void platform_reboot(void);

// Platform capabilities
typedef struct {
    uint8_t has_keyboard;
    uint8_t has_mouse;
    uint8_t has_touchscreen;
    uint8_t has_wifi;
    uint8_t has_bluetooth;
    uint8_t has_cellular;
    uint8_t has_gps;
    uint8_t has_sensors;
    uint8_t has_camera;
    uint8_t has_battery;
    uint8_t is_mobile;
    uint8_t supports_rotation;
} platform_capabilities_t;

void platform_get_capabilities(platform_capabilities_t* caps);

#endif