#ifndef AI_VOICE_ASSISTANT_H
#define AI_VOICE_ASSISTANT_H

#include <stdint.h>

// Voice command types
#define VOICE_COMMAND_UNKNOWN   0
#define VOICE_COMMAND_INSTALL   1
#define VOICE_COMMAND_SYSTEM    2
#define VOICE_COMMAND_QUERY     3
#define VOICE_COMMAND_CONTROL   4

typedef struct {
    int type;
    char parameters[256];
    float confidence;
} ai_command_intent_t;

typedef struct {
    int listening;
    float confidence_threshold;
    char last_command[512];
    uint64_t last_interaction;
} ai_voice_context_t;

void ai_voice_assistant_init(void);
void ai_voice_start_listening(void);
void ai_voice_stop_listening(void);
int ai_voice_process_command(const char* command);
void ai_voice_speak(const char* text);

static ai_command_intent_t ai_parse_command_intent(const char* command);
static int ai_voice_handle_install_command(const ai_command_intent_t* intent);
static int ai_voice_handle_system_command(const ai_command_intent_t* intent);
static int ai_voice_handle_query_command(const ai_command_intent_t* intent);
static int ai_voice_handle_control_command(const ai_command_intent_t* intent);

static void ai_voice_recognition_init(void);
static void ai_nlp_init(void);
static void ai_tts_init(void);

#endif