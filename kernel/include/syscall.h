#ifndef SYSCALL_H
#define SYSCALL_H

#include <stdint.h>

void syscall_init(void);
int handle_syscall(uint64_t syscall_number, void* params);

// System call numbers
#define SYS_EXIT    1
#define SYS_READ    2
#define SYS_WRITE   3
#define SYS_OPEN    4
#define SYS_CLOSE   5
#define SYS_MMAP    6
#define SYS_MUNMAP  7
#define SYS_FORK    8
#define SYS_EXEC    9
#define SYS_WAIT    10

#endif