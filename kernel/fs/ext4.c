#include <stdint.h>
#include <stddef.h>
#include "../core/kprintf.h"

#define EXT4_SUPER_MAGIC    0xEF53
#define EXT4_BLOCK_SIZE     4096
#define EXT4_ROOT_INO       2

typedef struct {
    uint32_t magic;
    uint32_t blocks_count;
    uint32_t inodes_count;
    uint32_t block_size;
    uint32_t inode_size;
    uint8_t  uuid[16];
    char     volume_name[16];
    uint32_t last_check;
    uint32_t checkinterval;
    uint32_t creator_os;
    uint32_t first_ino;
    uint16_t inode_size;
    uint16_t block_group_nr;
    uint32_t feature_compat;
    uint32_t feature_incompat;
    uint32_t feature_ro_compat;
} ext4_superblock_t;

typedef struct {
    uint16_t mode;
    uint16_t uid;
    uint32_t size;
    uint32_t atime;
    uint32_t ctime;
    uint32_t mtime;
    uint32_t dtime;
    uint16_t gid;
    uint16_t links_count;
    uint32_t blocks;
    uint32_t flags;
    uint32_t block[15];
    uint32_t generation;
    uint32_t file_acl;
    uint32_t dir_acl;
    uint32_t faddr;
    uint8_t  osd2[12];
} ext4_inode_t;

typedef struct {
    uint32_t block_bitmap;
    uint32_t inode_bitmap;
    uint32_t inode_table;
    uint16_t free_blocks_count;
    uint16_t free_inodes_count;
    uint16_t used_dirs_count;
    uint16_t pad;
    uint8_t  reserved[12];
} ext4_group_desc_t;

static ext4_superblock_t* superblock = NULL;
static ext4_group_desc_t* group_desc_table = NULL;
static uint32_t groups_count;
static uint32_t blocks_per_group;
static uint32_t inodes_per_group;

void ext4_init(void) {
    kprintf("Initializing EXT4 file system...\n");
    
    // Allocate superblock
    superblock = (ext4_superblock_t*)kmalloc(sizeof(ext4_superblock_t));
    if (!superblock) {
        kprintf("Failed to allocate superblock\n");
        return;
    }
    
    // Read superblock from disk (block 1)
    // TODO: Implement actual disk reading
    
    // Validate superblock
    if (superblock->magic != EXT4_SUPER_MAGIC) {
        kprintf("Invalid EXT4 superblock magic\n");
        kfree(superblock);
        superblock = NULL;
        return;
    }
    
    // Calculate filesystem parameters
    blocks_per_group = superblock->blocks_count / superblock->block_group_nr;
    inodes_per_group = superblock->inodes_count / superblock->block_group_nr;
    groups_count = (superblock->blocks_count + blocks_per_group - 1) / blocks_per_group;
    
    // Allocate group descriptor table
    size_t gdt_size = groups_count * sizeof(ext4_group_desc_t);
    group_desc_table = (ext4_group_desc_t*)kmalloc(gdt_size);
    if (!group_desc_table) {
        kprintf("Failed to allocate group descriptor table\n");
        kfree(superblock);
        superblock = NULL;
        return;
    }
    
    // Read group descriptor table
    // TODO: Implement actual disk reading
    
    kprintf("EXT4 file system initialized\n");
    kprintf("Volume: %s\n", superblock->volume_name);
    kprintf("Block size: %d bytes\n", superblock->block_size);
    kprintf("Inode size: %d bytes\n", superblock->inode_size);
    kprintf("Groups: %d\n", groups_count);
}

int ext4_read_inode(uint32_t inode_num, ext4_inode_t* inode) {
    if (!superblock || !group_desc_table || !inode) {
        return -1;
    }
    
    uint32_t group = (inode_num - 1) / inodes_per_group;
    uint32_t index = (inode_num - 1) % inodes_per_group;
    
    if (group >= groups_count) {
        return -1;
    }
    
    uint32_t block = group_desc_table[group].inode_table + 
                     (index * sizeof(ext4_inode_t)) / superblock->block_size;
    uint32_t offset = (index * sizeof(ext4_inode_t)) % superblock->block_size;
    
    // TODO: Read inode from disk
    // For now, just zero out the inode
    memset(inode, 0, sizeof(ext4_inode_t));
    
    return 0;
}

int ext4_write_inode(uint32_t inode_num, const ext4_inode_t* inode) {
    if (!superblock || !group_desc_table || !inode) {
        return -1;
    }
    
    uint32_t group = (inode_num - 1) / inodes_per_group;
    uint32_t index = (inode_num - 1) % inodes_per_group;
    
    if (group >= groups_count) {
        return -1;
    }
    
    uint32_t block = group_desc_table[group].inode_table + 
                     (index * sizeof(ext4_inode_t)) / superblock->block_size;
    uint32_t offset = (index * sizeof(ext4_inode_t)) % superblock->block_size;
    
    // TODO: Write inode to disk
    
    return 0;
}

int ext4_allocate_block(void) {
    if (!superblock || !group_desc_table) {
        return -1;
    }
    
    // Simple first-fit block allocation
    for (uint32_t group = 0; group < groups_count; group++) {
        if (group_desc_table[group].free_blocks_count > 0) {
            // TODO: Search block bitmap for free block
            // TODO: Mark block as used
            // TODO: Update group descriptor
            // TODO: Write changes to disk
            return -1; // Temporary
        }
    }
    
    return -1;
}

void ext4_free_block(uint32_t block_num) {
    if (!superblock || !group_desc_table) {
        return;
    }
    
    uint32_t group = block_num / blocks_per_group;
    uint32_t index = block_num % blocks_per_group;
    
    if (group >= groups_count) {
        return;
    }
    
    // TODO: Update block bitmap
    // TODO: Update group descriptor
    // TODO: Write changes to disk
}

// Directory entry structure
typedef struct {
    uint32_t inode;
    uint16_t rec_len;
    uint8_t  name_len;
    uint8_t  file_type;
    char     name[];
} ext4_dir_entry_t;

int ext4_create_file(const char* path, uint16_t mode) {
    // TODO: Implement file creation
    return -1;
}

int ext4_read_file(uint32_t inode_num, void* buffer, size_t size, size_t offset) {
    // TODO: Implement file reading
    return -1;
}

int ext4_write_file(uint32_t inode_num, const void* buffer, size_t size, size_t offset) {
    // TODO: Implement file writing
    return -1;
}

int ext4_delete_file(const char* path) {
    // TODO: Implement file deletion
    return -1;
}