#ifndef PSF_H
#define PSF_H
#include <stdint.h>
#include "../../Include/com.h"
#include "../../Include/string.h"

typedef struct {
    uint32_t magic;       // Magic number (0x864AB572 untuk PSF v2)
    uint32_t version;     // Version (2 untuk PSF v2)
    uint32_t headersize;  // Ukuran header
    uint32_t flags;       // Flags
    uint32_t glyph_count; // Jumlah glyph
    uint32_t glyph_size;  // Ukuran setiap glyph
    uint32_t glyph_width; // Lebar glyph
    uint32_t glyph_height;// Tinggi glyph
} PSFHeader;

void* load_psf_file(const char *path);
void* parse_psf(void *buffer);

#endif