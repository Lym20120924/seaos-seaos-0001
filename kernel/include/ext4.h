#ifndef EXT4_H
#define EXT4_H

#include <stdint.h>
#include <stddef.h>

// File system structures
typedef struct ext4_inode ext4_inode_t;
typedef struct ext4_dir_entry ext4_dir_entry_t;

// Initialization
void ext4_init(void);

// Inode operations
int ext4_read_inode(uint32_t inode_num, ext4_inode_t* inode);
int ext4_write_inode(uint32_t inode_num, const ext4_inode_t* inode);

// Block operations
int ext4_allocate_block(void);
void ext4_free_block(uint32_t block_num);

// File operations
int ext4_create_file(const char* path, uint16_t mode);
int ext4_read_file(uint32_t inode_num, void* buffer, size_t size, size_t offset);
int ext4_write_file(uint32_t inode_num, const void* buffer, size_t size, size_t offset);
int ext4_delete_file(const char* path);

#endif