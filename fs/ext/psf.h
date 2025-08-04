#ifndef PSF_H
#define PSF_H
#include <unostype.h>

#define PSF1_MAGIC0 0x36
#define PSF1_MAGIC1 0x04
#define PSF2_MAGIC 0x864AB572

typedef enum {
    PSF_TYPE_NONE = 0,
    PSF_TYPE_1,
    PSF_TYPE_2
} PSFType;

typedef struct {
    uint32_t glyph_count;
    uint32_t glyph_size;
    uint32_t glyph_width;
    uint32_t glyph_height;
    uint32_t headersize;
    const uint8_t* glyph_data;
} PSFHeader;

typedef struct {
    uint32_t magic;       // Magic number (0x864AB572 untuk PSF v2)
    uint32_t version;     // Version (2 untuk PSF v2)
    uint32_t headersize;  // Ukuran header
    uint32_t flags;       // Flags
    uint32_t glyph_count; // Jumlah glyph
    uint32_t glyph_size;  // Ukuran setiap glyph
    uint32_t glyph_width; // Lebar glyph
    uint32_t glyph_height;// Tinggi glyph
} __attribute__((packed))PSF2Header;

typedef struct {
    uint8_t magic[2];     // 0x36, 0x04
    uint8_t mode;         // 0 = 256 glyphs, 1 = 512 glyphs
    uint8_t charsize;     // Tinggi glyph (misalnya 16 untuk 8x16)
} __attribute__((packed)) PSF1Header;

extern void *system_font_psf;

void* load_psf_file(const char *path);
PSFHeader *parse_psf(void *buffer);
void DrawSimplePSFText(const char *text, int x, int y, uint32_t color);

#endif