#ifndef MOBILE_TELEPHONY_H
#define MOBILE_TELEPHONY_H

#include <stdint.h>

// Call states
#define CALL_STATE_IDLE       0
#define CALL_STATE_DIALING    1
#define CALL_STATE_RINGING    2
#define CALL_STATE_ACTIVE     3
#define CALL_STATE_HOLDING    4

// Network types
#define NETWORK_TYPE_UNKNOWN  0
#define NETWORK_TYPE_2G       1
#define NETWORK_TYPE_3G       2
#define NETWORK_TYPE_4G       3
#define NETWORK_TYPE_5G       4

void telephony_init(void);
int telephony_init_modem(void);
int telephony_is_sim_present(void);
int telephony_get_operator(char* name, size_t size);
int telephony_get_signal_strength(void);
int telephony_get_network_type(void);

int telephony_make_call(const char* number);
void telephony_end_call(void);
void telephony_answer_call(void);
void telephony_reject_call(void);
int telephony_get_call_state(void);

int telephony_send_sms(const char* number, const char* message);
void telephony_enable_radio(void);
void telephony_disable_radio(void);

#endif