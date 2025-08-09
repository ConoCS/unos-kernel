#include <unoskrnl.h>

void *system_font_psf = NULL;
const uint8_t* system_glyph_data = NULL;
int glyph_width, glyph_height, glyph_size;
PSFHeader *system_psf_info = NULL;
INT term_cursor_x = 0;
INT term_cursor_y = 0;
INT GraphicOK = 0;

#define defaultcolor 0xFFFFFFFF

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
    GraphicOK = 1;
}

UNFUNCTION
DrawSimplePSFText(
    IN CONST CHARA8 *text,
    IN USINT32 Color
) {
    INT GlyphWidth = glyph_width;
    INT GlyphHeight = glyph_height;
    const int term_cols = screen_width / GlyphWidth;
    const int term_rows = screen_height / GlyphHeight;

    while (*text) {
        CHARA8 ch = *text;

        if (ch == '\n') {
            term_cursor_x = 0;
            term_cursor_y++;
        } else if (ch == '\r') {
            term_cursor_x = 0;
        } else {
            CONST USINT8 *glyph = system_glyph_data + (ch * glyph_size);
            draw_glyph(term_cursor_x * GlyphWidth, term_cursor_y * GlyphHeight,
                       glyph, GlyphWidth, GlyphHeight, Color);
            term_cursor_x++;
        }

        if (term_cursor_x >= term_cols) {
            term_cursor_x = 0;
            term_cursor_y++;
        }

        if (term_cursor_y >= term_rows) {
            TerminalTTYScrollUp(0x00000000); // scroll ke atas dengan background hitam
            delay(50);
            term_cursor_y = term_rows - 1;   // jaga cursor tetap di baris paling bawah
        }

        text++;
    }
}

UNFUNCTION
PSFPutChar(
    IN CHARA8 ch,
    IN USINT32 Color
)
{
    // Cek apakah data glyph belum siap
    if (system_glyph_data == NULL || glyph_size == 0 || screen_width == 0 || screen_height == 0)
        return; // Skip kalau belum diinisialisasi

    INT GlyphWidth = glyph_width;
    INT GlyphHeight = glyph_height;
    const int term_cols = screen_width / GlyphWidth;
    const int term_rows = screen_height / GlyphHeight;


    if(ch == '\n') {
        term_cursor_x = 0;
        term_cursor_y++;
    } else if (ch == '\r') {
        term_cursor_x = 0;
    } else if(ch == '\b') {
        if(term_cursor_x > 0) {
            term_cursor_x--;
        } else if (term_cursor_y > 0) {
            term_cursor_y--;
            term_cursor_x = term_cols - 1;
        }
        draw_glyph(term_cursor_x * glyph_width, term_cursor_y * glyph_height,
             system_glyph_data + (' ' * glyph_size), glyph_width,
              glyph_height, Color);
    } else {
        CONST USINT8 *glyph = system_glyph_data + (ch * glyph_size);
        draw_glyph(term_cursor_x * glyph_width, term_cursor_y * glyph_height, 
                    glyph, glyph_width, glyph_height, Color);
        term_cursor_x++;
    }

    if(term_cursor_x >= term_cols) {
        term_cursor_x = 0;
        term_cursor_y++;
    }

    if(term_cursor_y >= term_rows) {
        TerminalTTYScrollUp(0x00000000); // scroll ke atas dengan background hitam
        delay(50);
        term_cursor_y = term_rows - 1;   // jaga cursor tetap di baris paling bawah
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

    if ((size_t)read_bytes != file_size) {
        serial_printf("[Error] Gagal membaca file %s (read %d bytes)\n", path, read_bytes);
        return NULL;
    }

    serial_printf("[OK] File %s berhasil dibaca (%d bytes)\n", path, read_bytes);
    return buffer;
}

UNFUNCTION
PSFPrintf (
    IN CONST CHARA8 *Text,
    ...
) {
    va_list args;
    va_start(args, Text);

    while(*Text) {
        if(*Text == '%') {
            Text++;

            CHARA8 Buf[128];
            CONST CHARA8 *str = NULL;
            switch(*Text) {
                case 'd':
                case 'i': {
                    int val = va_arg(args, INT);
                    itoa(val, Buf);
                    DrawSimplePSFText(Buf, defaultcolor);
                    break;
                }
                case 's': {
                    str = va_arg(args, const char*);
                    DrawSimplePSFText(str, defaultcolor);
                    break;
                }
                case 'c': {
                    PSFPutChar((CHARA8)va_arg(args, int), defaultcolor);
                    break;
                }
                case 'u': {
                    unsigned int unsignedint = va_arg(args, unsigned int);
                    utoa((USINT64)unsignedint, Buf);
                    DrawSimplePSFText(Buf, defaultcolor);
                    break;
                }
                case 'x':
                case 'X': {
                    int uppercase = (*Text == 'X');
                    USINT64 val = va_arg(args, USINT64);
                    xtoa(val, Buf, uppercase);
                    DrawSimplePSFText("0x", defaultcolor);
                    DrawSimplePSFText(Buf, defaultcolor);
                }
                default:
                // Unknown format specifier, just print it raw
                PSFPutChar('%', defaultcolor);
                PSFPutChar(*Text, defaultcolor);
                break;
            }
        } else {
            PSFPutChar(*Text, defaultcolor);    
        }
        Text++;
    }
}

UNFUNCTION
ResetCursorX(

)
{
    term_cursor_x = 0;
}

UNFUNCTION
ResetCursorY(

)
{
    term_cursor_y = 0;
}

UNFUNCTION
TerminalTTYScrollUp(
    IN USINT32 Color_Background
) {
    INT bytes_per_pixel = 4;

    for(USINT32 y = 0; y < screen_height - glyph_height; y++) {
        memcpy(
            framebuffer + y * screen_width,
            framebuffer + (y + glyph_height) * screen_width,
            screen_width * bytes_per_pixel
        );
    }

    // Clear baris terakhir (paling bawah)
    for (USINT32 y = screen_height - glyph_height; y < screen_height; y++) {
        for (USINT32 x = 0; x < screen_width; x++) {
            framebuffer[y * screen_width + x] = Color_Background;
        }
    }

    term_cursor_y--;
}
