#include <stddef.h>
#include <stdint.h>
#include "kprintf.h"

#define PAGE_SIZE 4096
#define KERNEL_HEAP_START 0xC0000000
#define KERNEL_HEAP_INITIAL_SIZE (100 * 1024 * 1024) // 100MB initial heap

// Page flags
#define PAGE_PRESENT    (1 << 0)
#define PAGE_WRITABLE   (1 << 1)
#define PAGE_USER       (1 << 2)
#define PAGE_ALLOCATED  (1 << 3)

typedef struct page {
    uint32_t flags;
    struct page* next;
    uint32_t ref_count;
    void* virtual_addr;
} page_t;

typedef struct {
    page_t* free_pages;
    uint32_t total_pages;
    uint32_t used_pages;
    uint32_t kernel_pages;
    void* kernel_page_dir;
} page_allocator_t;

static page_allocator_t page_allocator;

// Memory regions for different purposes
static uint64_t* kernel_pml4 = NULL;  // Top level page table for x86_64
static uint64_t* kernel_pdpt = NULL;  // Page directory pointer table
static uint64_t* kernel_pd = NULL;    // Page directory
static uint64_t* kernel_pt = NULL;    // Page table

void mm_init(void) {
    kprintf("Initializing memory management...\n");
    
    // Initialize page allocator
    page_allocator.free_pages = NULL;
    page_allocator.total_pages = KERNEL_HEAP_INITIAL_SIZE / PAGE_SIZE;
    page_allocator.used_pages = 0;
    page_allocator.kernel_pages = 0;
    
    // Initialize paging structures
    kernel_pml4 = (uint64_t*)kmalloc_early(PAGE_SIZE);
    kernel_pdpt = (uint64_t*)kmalloc_early(PAGE_SIZE);
    kernel_pd = (uint64_t*)kmalloc_early(PAGE_SIZE);
    kernel_pt = (uint64_t*)kmalloc_early(PAGE_SIZE);
    
    if (!kernel_pml4 || !kernel_pdpt || !kernel_pd || !kernel_pt) {
        kprintf("Failed to allocate page tables\n");
        return;
    }
    
    // Clear page tables
    memset(kernel_pml4, 0, PAGE_SIZE);
    memset(kernel_pdpt, 0, PAGE_SIZE);
    memset(kernel_pd, 0, PAGE_SIZE);
    memset(kernel_pt, 0, PAGE_SIZE);
    
    // Set up initial page mappings
    kernel_pml4[0] = ((uint64_t)kernel_pdpt) | PAGE_PRESENT | PAGE_WRITABLE;
    kernel_pdpt[0] = ((uint64_t)kernel_pd) | PAGE_PRESENT | PAGE_WRITABLE;
    kernel_pd[0] = ((uint64_t)kernel_pt) | PAGE_PRESENT | PAGE_WRITABLE;
    
    // Identity map first 2MB for kernel
    for (uint64_t i = 0; i < 512; i++) {
        kernel_pt[i] = (i * PAGE_SIZE) | PAGE_PRESENT | PAGE_WRITABLE;
    }
    
    // Set up initial free page list
    for (uint32_t i = 0; i < page_allocator.total_pages; i++) {
        page_t* page = (page_t*)(KERNEL_HEAP_START + i * PAGE_SIZE);
        page->flags = 0;
        page->next = page_allocator.free_pages;
        page->ref_count = 0;
        page->virtual_addr = (void*)(KERNEL_HEAP_START + i * PAGE_SIZE);
        page_allocator.free_pages = page;
    }
    
    // Enable paging
    __asm__ volatile(
        "movq %0, %%cr3\n"
        :
        : "r"(kernel_pml4)
        : "memory"
    );
    
    kprintf("Memory management initialized\n");
    kprintf("Total pages: %d\n", page_allocator.total_pages);
    kprintf("Page size: %d bytes\n", PAGE_SIZE);
}

static void* kmalloc_early(size_t size) {
    static uintptr_t placement_address = KERNEL_HEAP_START;
    uintptr_t tmp = placement_address;
    placement_address += size;
    return (void*)tmp;
}

void* kmalloc(size_t size) {
    if (size == 0) {
        return NULL;
    }
    
    // Calculate number of pages needed
    uint32_t pages = (size + PAGE_SIZE - 1) / PAGE_SIZE;
    
    // Find continuous free pages
    page_t* start = NULL;
    page_t* current = page_allocator.free_pages;
    uint32_t continuous = 0;
    
    while (current != NULL) {
        if (!(current->flags & PAGE_ALLOCATED)) {
            if (continuous == 0) {
                start = current;
            }
            continuous++;
            if (continuous >= pages) {
                break;
            }
        } else {
            continuous = 0;
        }
        current = current->next;
    }
    
    if (continuous < pages) {
        return NULL;  // Not enough continuous pages
    }
    
    // Allocate the pages
    current = start;
    for (uint32_t i = 0; i < pages; i++) {
        current->flags |= PAGE_ALLOCATED;
        current->ref_count = 1;
        current = current->next;
    }
    
    page_allocator.used_pages += pages;
    
    // Return the virtual address of the first page
    return start->virtual_addr;
}

void kfree(void* ptr) {
    if (!ptr) {
        return;
    }
    
    // Find the page containing this address
    page_t* page = (page_t*)((uintptr_t)ptr & ~(PAGE_SIZE - 1));
    
    // Verify it's a valid allocated page
    if (!(page->flags & PAGE_ALLOCATED)) {
        kprintf("Warning: Attempting to free unallocated memory at %p\n", ptr);
        return;
    }
    
    // Decrease reference count
    page->ref_count--;
    
    // If reference count reaches 0, free the page
    if (page->ref_count == 0) {
        page->flags &= ~PAGE_ALLOCATED;
        page_allocator.used_pages--;
    }
}

// Virtual memory mapping functions
int map_page(void* virtual_addr, void* physical_addr, uint32_t flags) {
    uint64_t vaddr = (uint64_t)virtual_addr;
    uint64_t paddr = (uint64_t)physical_addr;
    
    uint64_t pml4_index = (vaddr >> 39) & 0x1FF;
    uint64_t pdpt_index = (vaddr >> 30) & 0x1FF;
    uint64_t pd_index = (vaddr >> 21) & 0x1FF;
    uint64_t pt_index = (vaddr >> 12) & 0x1FF;
    
    // Ensure page tables exist
    if (!(kernel_pml4[pml4_index] & PAGE_PRESENT)) {
        uint64_t* new_pdpt = (uint64_t*)kmalloc_early(PAGE_SIZE);
        if (!new_pdpt) return -1;
        memset(new_pdpt, 0, PAGE_SIZE);
        kernel_pml4[pml4_index] = ((uint64_t)new_pdpt) | PAGE_PRESENT | PAGE_WRITABLE;
    }
    
    uint64_t* pdpt = (uint64_t*)(kernel_pml4[pml4_index] & ~0xFFF);
    if (!(pdpt[pdpt_index] & PAGE_PRESENT)) {
        uint64_t* new_pd = (uint64_t*)kmalloc_early(PAGE_SIZE);
        if (!new_pd) return -1;
        memset(new_pd, 0, PAGE_SIZE);
        pdpt[pdpt_index] = ((uint64_t)new_pd) | PAGE_PRESENT | PAGE_WRITABLE;
    }
    
    uint64_t* pd = (uint64_t*)(pdpt[pdpt_index] & ~0xFFF);
    if (!(pd[pd_index] & PAGE_PRESENT)) {
        uint64_t* new_pt = (uint64_t*)kmalloc_early(PAGE_SIZE);
        if (!new_pt) return -1;
        memset(new_pt, 0, PAGE_SIZE);
        pd[pd_index] = ((uint64_t)new_pt) | PAGE_PRESENT | PAGE_WRITABLE;
    }
    
    uint64_t* pt = (uint64_t*)(pd[pd_index] & ~0xFFF);
    pt[pt_index] = paddr | flags;
    
    // Invalidate TLB entry
    __asm__ volatile("invlpg (%0)" : : "r"(virtual_addr) : "memory");
    
    return 0;
}

void unmap_page(void* virtual_addr) {
    uint64_t vaddr = (uint64_t)virtual_addr;
    
    uint64_t pml4_index = (vaddr >> 39) & 0x1FF;
    uint64_t pdpt_index = (vaddr >> 30) & 0x1FF;
    uint64_t pd_index = (vaddr >> 21) & 0x1FF;
    uint64_t pt_index = (vaddr >> 12) & 0x1FF;
    
    if (!(kernel_pml4[pml4_index] & PAGE_PRESENT)) {
        return;
    }
    
    uint64_t* pdpt = (uint64_t*)(kernel_pml4[pml4_index] & ~0xFFF);
    if (!(pdpt[pdpt_index] & PAGE_PRESENT)) {
        return;
    }
    
    uint64_t* pd = (uint64_t*)(pdpt[pdpt_index] & ~0xFFF);
    if (!(pd[pd_index] & PAGE_PRESENT)) {
        return;
    }
    
    uint64_t* pt = (uint64_t*)(pd[pd_index] & ~0xFFF);
    pt[pt_index] = 0;
    
    // Invalidate TLB entry
    __asm__ volatile("invlpg (%0)" : : "r"(virtual_addr) : "memory");
}