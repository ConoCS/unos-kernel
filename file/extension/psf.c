#include "psf.h"
#include "../../Include/graphic.h"
#include "../../Include/paging.h"
#include "../../Include/com.h"
#include "../../filesystem/vfs.h"

void* parse_psf(void *buffer) {
    PSFHeader *header = (PSFHeader*)buffer;
    serial_printf("[DEBUG] PSF Header Address: %p\n", header);
    serial_printf("[DEBUG] PSF Header Magic: 0x%X\n", header->magic);
    serial_printf("[DEBUG] PSF Header Glyph Count: %d\n", header->glyph_count);
    serial_printf("[DEBUG] PSF Header Glyph Size: %d\n", header->glyph_size);
    serial_printf("[DEBUG] PSF Header Glyph Dimensions: %dx%d\n", header->glyph_width, header->glyph_height);

    if (header->magic != 0x864AB572) {
        serial_printf("[Error] File PSF tidak valid\n");
        return NULL;
    }

    serial_printf("[OK] PSF file valid: %d glyphs, %dx%d size\n",
                  header->glyph_count, header->glyph_width, header->glyph_height);

    return (void*)((uint8_t*)buffer + header->headersize); // Return pointer ke data glyph
}

void* load_psf_file(const char *path) {
    serial_printf("[DEBUG] Loading PSF file: %s\n", path);

    VFSNode *file = Fat32Open(vfs_root, path);
    if (!file || file->type != VFS_TYPE_FILE) {
        serial_printf("[Error] File %s tidak ditemukan atau bukan file\n", path);
        return NULL;
    }

    serial_printf("[DEBUG] File size: %llu bytes\n", file->size);

    size_t file_size = file->size;
    void *buffer = kmalloc(file_size);
    if (!buffer) {
        serial_printf("[Error] Gagal mengalokasikan memori untuk file %s\n", path);
        return NULL;
    }

    serial_printf("[DEBUG] Memory allocated at: %p\n", buffer);

    int read_bytes = Fat32Read(file, 0, buffer, file_size);
    serial_printf("[DEBUG] Bytes read: %d\n", read_bytes);

    if (read_bytes != file_size) {
        serial_printf("[Error] Gagal membaca file %s (read %d bytes)\n", path, read_bytes);
        return NULL;
    }

    serial_printf("[OK] File %s berhasil dibaca (%d bytes)\n", path, read_bytes);
    return buffer;
}