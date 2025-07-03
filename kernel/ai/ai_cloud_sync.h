#ifndef AI_CLOUD_SYNC_H
#define AI_CLOUD_SYNC_H

#include <stdint.h>
#include "../net/tcp.h"

typedef struct {
    char device_id[64];
    int sync_enabled;
    uint64_t last_sync;
    uint32_t sync_interval;
    char cloud_token[256];
} ai_cloud_profile_t;

void ai_cloud_sync_init(void);
int ai_cloud_connect(void);
int ai_cloud_sync_profile(void);
int ai_cloud_download_updates(void);
int ai_cloud_upload_telemetry(void);
void ai_cloud_enable_sync(int enable);

static void ai_generate_device_id(char* buffer, size_t size);
static int ai_cloud_authenticate(tcp_connection_t* conn);

#endif