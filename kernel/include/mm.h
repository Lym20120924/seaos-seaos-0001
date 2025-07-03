#ifndef MM_H
#define MM_H

#include <stddef.h>
#include <stdint.h>

// Memory management initialization
void mm_init(void);

// Memory allocation functions
void* kmalloc(size_t size);
void kfree(void* ptr);

// Page mapping functions
int map_page(void* virtual_addr, void* physical_addr, uint32_t flags);
void unmap_page(void* virtual_addr);

// Page flags
#define PAGE_PRESENT    (1 << 0)
#define PAGE_WRITABLE   (1 << 1)
#define PAGE_USER       (1 << 2)
#define PAGE_ALLOCATED  (1 << 3)

#endif