#include <stdint.h>
#include <stddef.h>
#include "../core/kprintf.h"
#include "../core/mm.h"

#define NTFS_SIGNATURE    "NTFS    "
#define NTFS_SECTOR_SIZE  512
#define MFT_RECORD_SIZE   1024

// NTFS Boot Sector
typedef struct {
    uint8_t  jump[3];
    uint8_t  signature[8];
    uint16_t bytes_per_sector;
    uint8_t  sectors_per_cluster;
    uint16_t reserved_sectors;
    uint8_t  unused1[3];
    uint16_t unused2;
    uint8_t  media_descriptor;
    uint16_t unused3;
    uint16_t sectors_per_track;
    uint16_t number_of_heads;
    uint32_t hidden_sectors;
    uint32_t unused4;
    uint32_t unused5;
    uint64_t total_sectors;
    uint64_t mft_cluster;
    uint64_t mft_mirror_cluster;
    int8_t   clusters_per_mft_record;
    uint8_t  unused6[3];
    int8_t   clusters_per_index_record;
    uint8_t  unused7[3];
    uint64_t volume_serial;
    uint32_t checksum;
} __attribute__((packed)) ntfs_boot_sector_t;

// NTFS File Record Header
typedef struct {
    uint32_t magic;
    uint16_t usa_offset;
    uint16_t usa_count;
    uint64_t lsn;
    uint16_t sequence_number;
    uint16_t hard_link_count;
    uint16_t attrs_offset;
    uint16_t flags;
    uint32_t bytes_in_use;
    uint32_t bytes_allocated;
    uint64_t base_mft_record;
    uint16_t next_attr_id;
    uint16_t padding;
    uint32_t mft_record_number;
} __attribute__((packed)) ntfs_file_record_t;

// NTFS Attribute Header
typedef struct {
    uint32_t type;
    uint32_t length;
    uint8_t  non_resident;
    uint8_t  name_length;
    uint16_t name_offset;
    uint16_t flags;
    uint16_t attribute_id;
    union {
        struct {
            uint32_t value_length;
            uint16_t value_offset;
            uint8_t  resident_flags;
            uint8_t  padding;
        } resident;
        struct {
            uint64_t lowest_vcn;
            uint64_t highest_vcn;
            uint16_t mapping_pairs_offset;
            uint8_t  compression_unit;
            uint8_t  padding[5];
            uint64_t allocated_size;
            uint64_t data_size;
            uint64_t initialized_size;
            uint64_t compressed_size;
        } non_resident;
    };
} __attribute__((packed)) ntfs_attr_header_t;

static ntfs_boot_sector_t* boot_sector = NULL;
static uint64_t mft_start = 0;
static uint32_t cluster_size = 0;

void ntfs_init(void) {
    kprintf("Initializing NTFS driver...\n");
    
    // Allocate boot sector
    boot_sector = (ntfs_boot_sector_t*)kmalloc(sizeof(ntfs_boot_sector_t));
    if (!boot_sector) {
        kprintf("Failed to allocate NTFS boot sector\n");
        return;
    }
    
    // Read boot sector from disk
    // TODO: Implement disk reading
    
    // Verify NTFS signature
    if (memcmp(boot_sector->signature, NTFS_SIGNATURE, 8) != 0) {
        kprintf("Invalid NTFS signature\n");
        kfree(boot_sector);
        boot_sector = NULL;
        return;
    }
    
    // Calculate important values
    cluster_size = boot_sector->sectors_per_cluster * boot_sector->bytes_per_sector;
    mft_start = boot_sector->mft_cluster * cluster_size;
    
    kprintf("NTFS filesystem initialized\n");
    kprintf("Bytes per sector: %d\n", boot_sector->bytes_per_sector);
    kprintf("Sectors per cluster: %d\n", boot_sector->sectors_per_cluster);
    kprintf("MFT location: %lld\n", mft_start);
}

static int read_mft_record(uint64_t record_number, ntfs_file_record_t* record) {
    if (!boot_sector || !record) {
        return -1;
    }
    
    uint64_t record_offset = mft_start + (record_number * MFT_RECORD_SIZE);
    
    // TODO: Read MFT record from disk
    // For now, just zero out the record
    memset(record, 0, sizeof(ntfs_file_record_t));
    
    return 0;
}

static int write_mft_record(uint64_t record_number, const ntfs_file_record_t* record) {
    if (!boot_sector || !record) {
        return -1;
    }
    
    uint64_t record_offset = mft_start + (record_number * MFT_RECORD_SIZE);
    
    // TODO: Write MFT record to disk
    
    return 0;
}

int ntfs_read_file(uint64_t mft_number, void* buffer, size_t size, size_t offset) {
    if (!boot_sector || !buffer) {
        return -1;
    }
    
    ntfs_file_record_t* record = (ntfs_file_record_t*)kmalloc(MFT_RECORD_SIZE);
    if (!record) {
        return -1;
    }
    
    if (read_mft_record(mft_number, record) != 0) {
        kfree(record);
        return -1;
    }
    
    // TODO: Parse file attributes
    // TODO: Read data runs
    // TODO: Copy data to buffer
    
    kfree(record);
    return 0;
}

int ntfs_write_file(uint64_t mft_number, const void* buffer, size_t size, size_t offset) {
    if (!boot_sector || !buffer) {
        return -1;
    }
    
    ntfs_file_record_t* record = (ntfs_file_record_t*)kmalloc(MFT_RECORD_SIZE);
    if (!record) {
        return -1;
    }
    
    if (read_mft_record(mft_number, record) != 0) {
        kfree(record);
        return -1;
    }
    
    // TODO: Update file attributes
    // TODO: Allocate new clusters if needed
    // TODO: Write data runs
    // TODO: Update MFT record
    
    kfree(record);
    return 0;
}

int ntfs_create_file(const char* name, uint32_t attributes) {
    if (!boot_sector || !name) {
        return -1;
    }
    
    // TODO: Find free MFT record
    // TODO: Initialize file record
    // TODO: Create filename attribute
    // TODO: Create data attribute
    // TODO: Update parent directory
    
    return 0;
}

int ntfs_delete_file(uint64_t mft_number) {
    if (!boot_sector) {
        return -1;
    }
    
    ntfs_file_record_t* record = (ntfs_file_record_t*)kmalloc(MFT_RECORD_SIZE);
    if (!record) {
        return -1;
    }
    
    if (read_mft_record(mft_number, record) != 0) {
        kfree(record);
        return -1;
    }
    
    // TODO: Free allocated clusters
    // TODO: Mark MFT record as unused
    // TODO: Update parent directory
    
    kfree(record);
    return 0;
}

// NTFS directory operations
typedef struct {
    uint64_t parent_mft;
    uint64_t current_mft;
    uint32_t current_index;
} ntfs_dir_ctx_t;

int ntfs_open_dir(const char* path, ntfs_dir_ctx_t* ctx) {
    if (!boot_sector || !path || !ctx) {
        return -1;
    }
    
    // TODO: Parse path
    // TODO: Find directory MFT record
    // TODO: Initialize directory context
    
    return 0;
}

int ntfs_read_dir(ntfs_dir_ctx_t* ctx, char* name, size_t size) {
    if (!boot_sector || !ctx || !name) {
        return -1;
    }
    
    // TODO: Read directory entries
    // TODO: Copy next filename to buffer
    
    return 0;
}

void ntfs_close_dir(ntfs_dir_ctx_t* ctx) {
    // Clean up directory context
}

// NTFS attribute operations
static int find_attribute(ntfs_file_record_t* record, uint32_t type, ntfs_attr_header_t* attr) {
    if (!record || !attr) {
        return -1;
    }
    
    uint8_t* p = (uint8_t*)record + record->attrs_offset;
    
    while (p < (uint8_t*)record + record->bytes_in_use) {
        ntfs_attr_header_t* current = (ntfs_attr_header_t*)p;
        
        if (current->type == type) {
            memcpy(attr, current, sizeof(ntfs_attr_header_t));
            return 0;
        }
        
        if (current->type == -1) {
            break;
        }
        
        p += current->length;
    }
    
    return -1;
}

static int add_attribute(ntfs_file_record_t* record, ntfs_attr_header_t* attr) {
    if (!record || !attr) {
        return -1;
    }
    
    // TODO: Find space for new attribute
    // TODO: Update record headers
    // TODO: Copy attribute data
    
    return 0;
}

static int remove_attribute(ntfs_file_record_t* record, uint32_t type) {
    if (!record) {
        return -1;
    }
    
    // TODO: Find attribute
    // TODO: Remove attribute data
    // TODO: Update record headers
    
    return 0;
}