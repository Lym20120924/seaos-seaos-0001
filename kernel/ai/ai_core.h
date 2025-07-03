#ifndef AI_CORE_H
#define AI_CORE_H

#include <stdint.h>

// AI system states
#define AI_STATE_INITIALIZING  0
#define AI_STATE_READY         1
#define AI_STATE_PROCESSING    2
#define AI_STATE_ERROR         3

// Device detection results
#define DEVICE_TYPE_UNKNOWN    0
#define DEVICE_TYPE_PC         1
#define DEVICE_TYPE_LAPTOP     2
#define DEVICE_TYPE_TABLET     3
#define DEVICE_TYPE_PHONE      4
#define DEVICE_TYPE_SERVER     5
#define DEVICE_TYPE_EMBEDDED   6

// Package types
#define PACKAGE_TYPE_UNKNOWN   0
#define PACKAGE_TYPE_EXE       1  // Windows executable
#define PACKAGE_TYPE_APK       2  // Android package
#define PACKAGE_TYPE_IPA       3  // iOS package
#define PACKAGE_TYPE_DEB       4  // Debian package
#define PACKAGE_TYPE_RPM       5  // Red Hat package
#define PACKAGE_TYPE_DMG       6  // macOS disk image
#define PACKAGE_TYPE_MSI       7  // Windows installer
#define PACKAGE_TYPE_APPX      8  // Windows Store app

// AI initialization
void ai_init(void);
int ai_get_state(void);

// Device detection
int ai_detect_device_type(void);
const char* ai_get_device_name(void);
void ai_analyze_hardware_capabilities(void);

// Package management
int ai_detect_package_type(const char* filename);
int ai_install_package(const char* package_path);
int ai_can_run_package(int package_type);
const char* ai_get_package_type_name(int package_type);

// AI decision making
int ai_should_install_package(const char* package_path);
int ai_get_installation_method(int package_type);
void ai_optimize_for_device(void);

// Learning and adaptation
void ai_learn_from_usage(void);
void ai_update_device_profile(void);
void ai_save_preferences(void);

#endif