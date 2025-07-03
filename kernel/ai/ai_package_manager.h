#ifndef AI_PACKAGE_MANAGER_H
#define AI_PACKAGE_MANAGER_H

#include <stdint.h>

void ai_package_manager_init(void);

// Security and validation
int ai_security_scan_package(const char* package_path);
int ai_check_system_resources(const char* package_path);
int ai_get_installation_recommendation(const char* package_path);

// Package installation functions
int ai_install_exe_package(const char* package_path);
int ai_install_apk_package(const char* package_path);
int ai_install_ipa_package(const char* package_path);
int ai_install_deb_package(const char* package_path);
int ai_install_rpm_package(const char* package_path);
int ai_install_msi_package(const char* package_path);
int ai_install_appx_package(const char* package_path);

// Learning functions
void ai_learn_from_installation(const char* package_path, int success);

#endif