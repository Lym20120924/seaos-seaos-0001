#include <stdint.h>

#define MAX_SYSCALLS 256

// System call handler type
typedef int (*syscall_handler_t)(void*);

// System call table
static syscall_handler_t syscall_table[MAX_SYSCALLS];

void syscall_init(void) {
    // Initialize system call table with default handlers
    for (int i = 0; i < MAX_SYSCALLS; i++) {
        syscall_table[i] = NULL;
    }
    
    // Register basic system calls
    // TODO: Add actual system call implementations
}

// System call handler
int handle_syscall(uint64_t syscall_number, void* params) {
    if (syscall_number >= MAX_SYSCALLS || syscall_table[syscall_number] == NULL) {
        return -1; // Invalid system call
    }
    
    return syscall_table[syscall_number](params);
}