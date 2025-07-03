#ifndef NTFS_H
#define NTFS_H

#include <stdint.h>
#include <stddef.h>

// NTFS context structure
typedef struct ntfs_dir_ctx ntfs_dir_ctx_t;

// Initialization
void ntfs_init(void);

// File operations
int ntfs_read_file(uint64_t mft_number, void* buffer, size_t size, size_t offset);
int ntfs_write_file(uint64_t mft_number, const void* buffer, size_t size, size_t offset);
int ntfs_create_file(const char* name, uint32_t attributes);
int ntfs_delete_file(uint64_t mft_number);

// Directory operations
int ntfs_open_dir(const char* path, ntfs_dir_ctx_t* ctx);
int ntfs_read_dir(ntfs_dir_ctx_t* ctx, char* name, size_t size);
void ntfs_close_dir(ntfs_dir_ctx_t* ctx);

#endif