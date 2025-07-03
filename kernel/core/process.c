#include <stdint.h>
#include "kprintf.h"
#include "mm.h"

#define MAX_PROCESSES 1024
#define PROCESS_NAME_MAX 32
#define STACK_SIZE (16 * 1024)  // 16KB stack

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

typedef struct {
    uint64_t rax, rbx, rcx, rdx;
    uint64_t rsi, rdi, rbp, rsp;
    uint64_t r8, r9, r10, r11;
    uint64_t r12, r13, r14, r15;
    uint64_t rip, rflags;
    uint64_t cs, ss, ds, es, fs, gs;
    uint64_t cr3;  // Page directory base
} cpu_state_t;

typedef struct process {
    uint32_t pid;
    uint32_t ppid;  // Parent process ID
    char name[PROCESS_NAME_MAX];
    uint32_t state;
    uint32_t priority;
    
    // Memory management
    void* stack;
    void* kernel_stack;
    void* page_directory;
    
    // CPU state
    cpu_state_t cpu_state;
    
    // Scheduling
    uint64_t quantum;
    uint64_t time_slice;
    uint64_t total_time;
    
    // List management
    struct process* next;
    struct process* prev;
} process_t;

typedef struct {
    process_t* processes[MAX_PROCESSES];
    process_t* current;
    uint32_t process_count;
    uint32_t next_pid;
    
    // Multiple run queues for different priorities
    process_t* run_queues[4];  // One queue per priority level
} process_manager_t;

static process_manager_t process_manager;

void process_init(void) {
    kprintf("Initializing process manager...\n");
    
    // Initialize process manager
    memset(&process_manager, 0, sizeof(process_manager_t));
    process_manager.next_pid = 1;  // PID 0 reserved for kernel
    
    // Create kernel process (PID 0)
    process_t* kernel_process = (process_t*)kmalloc(sizeof(process_t));
    if (!kernel_process) {
        kprintf("Failed to create kernel process\n");
        return;
    }
    
    memset(kernel_process, 0, sizeof(process_t));
    strncpy(kernel_process->name, "kernel", PROCESS_NAME_MAX-1);
    kernel_process->pid = 0;
    kernel_process->state = PROCESS_STATE_RUNNING;
    kernel_process->priority = PRIORITY_SYSTEM;
    
    // Set up kernel process memory
    kernel_process->page_directory = (void*)read_cr3();
    
    // Add to process manager
    process_manager.processes[0] = kernel_process;
    process_manager.current = kernel_process;
    process_manager.process_count = 1;
    
    kprintf("Process manager initialized\n");
}

process_t* create_process(const char* name) {
    if (process_manager.process_count >= MAX_PROCESSES) {
        kprintf("Error: Maximum process limit reached\n");
        return NULL;
    }
    
    // Allocate process structure
    process_t* process = (process_t*)kmalloc(sizeof(process_t));
    if (!process) {
        kprintf("Error: Failed to allocate process structure\n");
        return NULL;
    }
    
    // Initialize process
    memset(process, 0, sizeof(process_t));
    strncpy(process->name, name, PROCESS_NAME_MAX-1);
    process->pid = process_manager.next_pid++;
    process->ppid = process_manager.current->pid;
    process->state = PROCESS_STATE_NEW;
    process->priority = PRIORITY_NORMAL;
    
    // Allocate stack
    process->stack = kmalloc(STACK_SIZE);
    if (!process->stack) {
        kprintf("Error: Failed to allocate process stack\n");
        kfree(process);
        return NULL;
    }
    
    // Allocate kernel stack
    process->kernel_stack = kmalloc(STACK_SIZE);
    if (!process->kernel_stack) {
        kprintf("Error: Failed to allocate kernel stack\n");
        kfree(process->stack);
        kfree(process);
        return NULL;
    }
    
    // Create page directory
    process->page_directory = create_page_directory();
    if (!process->page_directory) {
        kprintf("Error: Failed to create page directory\n");
        kfree(process->kernel_stack);
        kfree(process->stack);
        kfree(process);
        return NULL;
    }
    
    // Initialize CPU state
    process->cpu_state.rsp = (uint64_t)process->stack + STACK_SIZE;
    process->cpu_state.rbp = process->cpu_state.rsp;
    process->cpu_state.rflags = 0x202;  // Enable interrupts
    process->cpu_state.cs = 0x08;  // Kernel code segment
    process->cpu_state.ss = 0x10;  // Kernel data segment
    process->cpu_state.cr3 = (uint64_t)process->page_directory;
    
    // Add to process manager
    process_manager.processes[process->pid] = process;
    process_manager.process_count++;
    
    // Add to appropriate run queue
    process->next = process_manager.run_queues[process->priority];
    if (process->next) {
        process->next->prev = process;
    }
    process_manager.run_queues[process->priority] = process;
    
    kprintf("Created process '%s' with PID %d\n", process->name, process->pid);
    return process;
}

void schedule(void) {
    if (process_manager.process_count <= 1) {
        return;  // Only kernel process exists
    }
    
    // Save current process state
    process_t* current = process_manager.current;
    save_cpu_state(&current->cpu_state);
    
    // Find next process to run
    process_t* next = NULL;
    
    // Check each priority level, starting from highest
    for (int priority = PRIORITY_SYSTEM; priority >= PRIORITY_LOW; priority--) {
        if (process_manager.run_queues[priority]) {
            next = process_manager.run_queues[priority];
            
            // Move to end of queue (round robin)
            process_manager.run_queues[priority] = next->next;
            if (next->next) {
                next->next->prev = NULL;
            }
            
            // Add current process back to queue if still runnable
            if (current->state == PROCESS_STATE_RUNNING) {
                current->state = PROCESS_STATE_READY;
                current->next = process_manager.run_queues[current->priority];
                if (current->next) {
                    current->next->prev = current;
                }
                process_manager.run_queues[current->priority] = current;
            }
            
            break;
        }
    }
    
    if (!next) {
        return;  // No runnable processes
    }
    
    // Switch to next process
    next->state = PROCESS_STATE_RUNNING;
    process_manager.current = next;
    
    // Load process state
    switch_page_directory(next->page_directory);
    restore_cpu_state(&next->cpu_state);
}

void exit_process(int status) {
    process_t* process = process_manager.current;
    
    // Clean up process resources
    kfree(process->stack);
    kfree(process->kernel_stack);
    destroy_page_directory(process->page_directory);
    
    // Remove from run queue
    if (process->prev) {
        process->prev->next = process->next;
    } else {
        process_manager.run_queues[process->priority] = process->next;
    }
    if (process->next) {
        process->next->prev = process->prev;
    }
    
    // Mark as zombie until parent collects status
    process->state = PROCESS_STATE_ZOMBIE;
    
    // Force schedule
    schedule();
}

// Architecture-specific functions
static void save_cpu_state(cpu_state_t* state) {
    __asm__ volatile(
        "movq %%rax, %0\n"
        "movq %%rbx, %1\n"
        "movq %%rcx, %2\n"
        "movq %%rdx, %3\n"
        "movq %%rsi, %4\n"
        "movq %%rdi, %5\n"
        "movq %%rbp, %6\n"
        "movq %%rsp, %7\n"
        "movq %%r8,  %8\n"
        "movq %%r9,  %9\n"
        "movq %%r10, %10\n"
        "movq %%r11, %11\n"
        "movq %%r12, %12\n"
        "movq %%r13, %13\n"
        "movq %%r14, %14\n"
        "movq %%r15, %15\n"
        : "=m"(state->rax), "=m"(state->rbx), "=m"(state->rcx), "=m"(state->rdx),
          "=m"(state->rsi), "=m"(state->rdi), "=m"(state->rbp), "=m"(state->rsp),
          "=m"(state->r8),  "=m"(state->r9),  "=m"(state->r10), "=m"(state->r11),
          "=m"(state->r12), "=m"(state->r13), "=m"(state->r14), "=m"(state->r15)
        :
        : "memory"
    );
    
    state->cr3 = read_cr3();
}

static void restore_cpu_state(const cpu_state_t* state) {
    __asm__ volatile(
        "movq %0, %%rax\n"
        "movq %1, %%rbx\n"
        "movq %2, %%rcx\n"
        "movq %3, %%rdx\n"
        "movq %4, %%rsi\n"
        "movq %5, %%rdi\n"
        "movq %6, %%rbp\n"
        "movq %7, %%rsp\n"
        "movq %8, %%r8\n"
        "movq %9, %%r9\n"
        "movq %10, %%r10\n"
        "movq %11, %%r11\n"
        "movq %12, %%r12\n"
        "movq %13, %%r13\n"
        "movq %14, %%r14\n"
        "movq %15, %%r15\n"
        :
        : "m"(state->rax), "m"(state->rbx), "m"(state->rcx), "m"(state->rdx),
          "m"(state->rsi), "m"(state->rdi), "m"(state->rbp), "m"(state->rsp),
          "m"(state->r8),  "m"(state->r9),  "m"(state->r10), "m"(state->r11),
          "m"(state->r12), "m"(state->r13), "m"(state->r14), "m"(state->r15)
        : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", "rbp", "rsp",
          "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15", "memory"
    );
    
    write_cr3(state->cr3);
}

static uint64_t read_cr3(void) {
    uint64_t cr3;
    __asm__ volatile("movq %%cr3, %0" : "=r"(cr3));
    return cr3;
}

static void write_cr3(uint64_t cr3) {
    __asm__ volatile("movq %0, %%cr3" : : "r"(cr3));
}