#ifndef AI_DEVICE_DETECTION_H
#define AI_DEVICE_DETECTION_H

#include <stdint.h>
#include "../include/platform.h"

// AI installation methods
#define AI_INSTALL_METHOD_UNKNOWN     0
#define AI_INSTALL_METHOD_EXECUTE     1
#define AI_INSTALL_METHOD_ADB         2
#define AI_INSTALL_METHOD_ITUNES      3
#define AI_INSTALL_METHOD_DPKG        4
#define AI_INSTALL_METHOD_RPM         5
#define AI_INSTALL_METHOD_MSIEXEC     6
#define AI_INSTALL_METHOD_POWERSHELL  7

typedef struct {
    uint32_t cpu_score;
    uint32_t memory_score;
    uint32_t storage_score;
    uint32_t graphics_score;
    uint32_t overall_score;
} ai_performance_metrics_t;

typedef struct {
    int device_type;
    platform_capabilities_t capabilities;
    ai_performance_metrics_t performance;
    uint64_t last_updated;
    char device_signature[64];
} ai_device_profile_t;

void ai_device_detection_init(void);
void ai_load_device_profile(ai_device_profile_t* profile);
void ai_save_device_profile(const ai_device_profile_t* profile);
void ai_measure_system_performance(ai_performance_metrics_t* metrics);

// AI optimization functions
void ai_enable_power_saving_mode(void);
void ai_optimize_for_touch_interface(void);
void ai_enable_mobile_features(void);
void ai_enable_rotation_support(void);
void ai_optimize_battery_usage(void);
void ai_enable_power_management(void);
void ai_optimize_for_mobility(void);
void ai_enable_performance_mode(void);
void ai_optimize_for_productivity(void);
void ai_enable_server_optimizations(void);
void ai_disable_unnecessary_services(void);

#endif