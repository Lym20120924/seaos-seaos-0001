#ifndef AI_SECURITY_SCANNER_H
#define AI_SECURITY_SCANNER_H

#include <stdint.h>

// Scan results
#define AI_SCAN_CLEAN       0
#define AI_SCAN_SUSPICIOUS  1
#define AI_SCAN_MALWARE     2
#define AI_SCAN_ERROR       3

// Threat levels
#define AI_THREAT_NONE      0
#define AI_THREAT_LOW       1
#define AI_THREAT_MEDIUM    2
#define AI_THREAT_HIGH      3
#define AI_THREAT_CRITICAL  4

typedef struct {
    int scan_result;
    int threat_level;
    char threat_name[64];
    char description[256];
} ai_scan_result_t;

typedef struct {
    uint32_t signature_count;
    uint32_t pattern_count;
    uint64_t last_update;
} ai_security_database_t;

void ai_security_scanner_init(void);
int ai_scan_file(const char* filepath);
int ai_scan_package(const char* package_path);
int ai_real_time_protection(const char* filepath);
void ai_update_security_database(void);

static void ai_load_virus_signatures(void);
static void ai_load_behavioral_patterns(void);
static void ai_init_security_ml_models(void);
static int ai_signature_scan(const char* filepath);
static int ai_behavioral_scan(const char* filepath);
static int ai_ml_scan(const char* filepath);
static int ai_quick_scan(const char* filepath);
static int ai_extract_package(const char* package_path, char* temp_dir);
static void ai_cleanup_temp_dir(const char* temp_dir);
static void ai_quarantine_file(const char* filepath);
static const char* ai_get_scan_result_name(int result);

#endif