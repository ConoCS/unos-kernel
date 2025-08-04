#include <unoskrnl.h>

void *system_font_psf = NULL;
const uint8_t* system_glyph_data = NULL;
int glyph_width, glyph_height, glyph_size;
PSFHeader *system_psf_info = NULL;

void InitPSFFontGraphic() {
    system_font_psf = load_psf_file("/UnOS/font/Lat15-VGA16.psf");
}

void PreparingGlobalPSFFont() {
    system_psf_info = parse_psf(system_font_psf);
    if(!system_psf_info) {
        serial_printf("[Error] Failed to parse PSF file\n");
        return;
    }

    glyph_width = system_psf_info->glyph_width;
    glyph_height = system_psf_info->glyph_height;
    glyph_size = system_psf_info->glyph_size;
    system_glyph_data = system_psf_info->glyph_data;
}

void DrawSimplePSFText(const char *text, int x, int y, uint32_t color) {
    while(*text) {
        char ch = *text;
        const uint8_t *glyph = system_glyph_data + (ch * glyph_size );
        draw_glyph(x, y, glyph, glyph_width, glyph_height, color);
        x += glyph_width;
        text++;
    }
}

PSFHeader *parse_psf(void *buffer) {
    uint8_t *buf = (uint8_t*)buffer;
    PSFHeader *info = (PSFHeader*)kmalloc(sizeof(PSFHeader));

    serial_printf("First 16 bytes of PSD File\n");
    for (int i = 0; i < 16; i++) {
        serial_printf("%x", buf[i]);
        if (i % 8 == 7) serial_print("\n");
    }

    uint8_t *magic16 = (uint8_t*)buf;
    if (magic16[0] == PSF1_MAGIC0 && magic16[1] == PSF1_MAGIC1) {
        PSF1Header *header = (PSF1Header*)buf;
        info->glyph_count = (header->mode == 0x01) ? 512 : 256;
        info->glyph_size = header->charsize;
        info->glyph_width = 8;
        info->glyph_height = header->charsize;
        info->headersize = sizeof(PSF1Header);
        info->glyph_data = buf + sizeof(PSF1Header);
        serial_printf("[DEBUG] PSF1 detected: glyph_count=%d, glyph_size=%d, glyph_width=%d, glyph_height=%d\n",
                      info->glyph_count, info->glyph_size, info->glyph_width, info->glyph_height);
        return info;
    }
    
    uint32_t magic32 = *(uint32_t*)buf;
    if (magic32 == PSF2_MAGIC) {
        PSF2Header *header = (PSF2Header*)buf;
        info->glyph_count = header->glyph_count;
        info->glyph_size = header->glyph_size;
        info->glyph_width = header->glyph_width;
        info->glyph_height = header->glyph_height;
        info->headersize = header->headersize;
        info->glyph_data = buf + header->headersize;
        serial_printf("[OK] Detected PSF2\n");
        return info;
    }

    serial_printf("[Error] File is not PSF1 nor PSF2\n");
    return NULL;
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