#ifndef _UNOS_MOUNT_VFS_
#define _UNOS_MOUNT_VFS_

#define VFS_TYPE_FILE 1
#define VFS_TYPE_DIR  2

#include <stdint.h>
#include "../driver/storage/fat32.h"

typedef struct VFSNode {
    char name[16];
    uint8_t type;           // 0=file, 1=dir
    uint32_t first_cluster;
    struct VFSNode* parent;
    struct VFSNode** children;
    uint32_t child_count;
    void* fs_ops;           // pointer ke fat32_ops
    void* fs_data;
} VFSNode;

typedef struct FSOperations {
    /// Lookup a child node by name in a directory.
    /// @param dir   Pointer to a directory VFSNode
    /// @param name  Null-terminated filename or subdirectory name
    /// @returns     Pointer to the child VFSNode, or NULL if not found
    struct VFSNode* (*lookup)(struct VFSNode *dir, const char *name);

    /// Read data from a file node.
    /// @param file    Pointer to the file VFSNode
    /// @param buf     Destination buffer
    /// @param len     Number of bytes to read
    /// @param offset  Offset within the file to start reading
    /// @returns       Number of bytes actually read, or negative on error
    int (*read)(struct VFSNode *file, void *buf, uint32_t len, uint32_t offset);

    /// Write data into a file node (for R/W filesystems).
    /// @param file    Pointer to the file VFSNode
    /// @param buf     Source buffer
    /// @param len     Number of bytes to write
    /// @param offset  Offset within the file to start writing
    /// @returns       Number of bytes actually written, or negative on error
    int (*write)(struct VFSNode *file, const void *buf, uint32_t len, uint32_t offset);

    /// Read directory entries (readdir). Fills an array of child pointers.
    /// @param dir         Pointer to a directory VFSNode
    /// @param out_nodes   Pointer to array where child pointers will be stored
    /// @param max_entries Maximum number of entries the array can hold
    /// @returns           Number of entries written, or negative on error
    int (*readdir)(struct VFSNode *dir, struct VFSNode **out_nodes, uint32_t max_entries);

    /// Optional: create a new file or directory under `dir` with given name.
    /// @param dir    Directory node to create in
    /// @param name   Name of the new entry
    /// @param is_dir Non-zero to create directory, zero to create file
    /// @returns      Pointer to the new VFSNode, or NULL on failure
    struct VFSNode* (*create)(struct VFSNode *dir, const char *name, int is_dir);

    /// Optional: remove a file or directory.
    /// @param node   Node to remove
    /// @returns      0 on success, negative on error
    int (*remove)(struct VFSNode *node);

} FSOperations;

extern FSOperations fat32_ops;

void VFSMountFAT32Root(HBA_PORT *port, VFSNode *mount_point);
VFSNode* Fat32Lookup(VFSNode *dir, const char *name);

#endif