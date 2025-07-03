#ifndef PROCESS_H
#define PROCESS_H

#include <stdint.h>

// Process structure
typedef struct process process_t;

// Process management functions
void process_init(void);
process_t* create_process(const char* name);
void schedule(void);
void exit_process(int status);

// Process states
#define PROCESS_STATE_NEW       0
#define PROCESS_STATE_READY     1
#define PROCESS_STATE_RUNNING   2
#define PROCESS_STATE_BLOCKED   3
#define PROCESS_STATE_ZOMBIE    4

// Process priorities
#define PRIORITY_LOW     0
#define PRIORITY_NORMAL  1
#define PRIORITY_HIGH    2
#define PRIORITY_SYSTEM  3

#endif