#include <unoskrnl.h>

FUNCWITHSTATUS ShowBGRT(IN ACPI_BGRT *Bgrt) {
    if (!Bgrt) {
        serial_printf("[Warning] BGRT Not Found\n");
        return STATUS_UNKNOWN;
    }

    serial_printf("[BGRT] Version: %u\n", Bgrt->Version);
    serial_printf("[BGRT] Status : %02X\n", Bgrt->Status);
    serial_printf("[BGRT] Image Type: %u", Bgrt->ImageType);
    serial_printf("[BGRT] Image Address   : 0x%08X%08X\n",
                  (USINT32)((USINT64)Bgrt->ImageAddress >> 32),
                  (USINT32)((USINT64)Bgrt->ImageAddress & 0xFFFFFFFF));
    serial_printf("[BGRT] Image Offset X: %u\n", Bgrt->ImageOffsetX);
    serial_printf("[BGRT] Image Offset Y: %u\n", Bgrt->ImageOffsetY);
    
    USINT8 *BmpData = (USINT8*)Bgrt->ImageAddress;

    if(BmpData[0] != 'B' || BmpData[1] != 'M') {
        serial_printf("[Error] BMP Invalid\n");
        return STATUS_INVALID;
    }

    USINT32 PixelOffset = *(USINT32*)&BmpData[0x0A];
    INT32 Width = *(USINT32*)&BmpData[0x12];
    INT32 Height = *(USINT32*)&BmpData[0x16];
    USINT16 BitCount = *(USINT16*)&BmpData[0x1C];

    if(BitCount == 32) {
        serial_printf("[BGRT] 32-BIT\n");
    } else if (BitCount == 24) {
        serial_printf("[BGRT] 24-BIT\n");
    }

    serial_printf("[INFO] BMP size: %dx%d, BitCount: %d\n", Width, Height, BitCount);

    USINT8 *Pixels = BmpData + PixelOffset;

    USINT32 Padding = (4 - (Width * (BitCount / 8)) % 4) % 4; // padding per row (BMP rows are aligned to 4 bytes)

    for (INT32 y = 0; y < Height; y++) {
        USINT8 *row = Pixels + (Height - 1 - y) * (Width * 3 + Padding);
        for(INT32 x = 0; x < Width; x++) {

            if((x + Bgrt->ImageOffsetX) >= screen_width || (y + Bgrt->ImageOffsetY) >= screen_height) {
                continue;
            }

            if(BitCount == 32) {
                USINT8 *PX = Pixels + (x + (Height - 1 - y) * Width) * 4;
                USINT32 Pixel = *(USINT32*)PX;
                put_pixel(x + Bgrt->ImageOffsetX, y + Bgrt->ImageOffsetY, Pixel);
            } else if (BitCount == 24) {
                        // pastikan di layar masih di dalam bounds
                USINT32 screen_x = x + Bgrt->ImageOffsetX;
                USINT32 screen_y = y + Bgrt->ImageOffsetY;
                if (screen_x >= screen_width || screen_y >= screen_height) 
                    continue;

                // ambil BGR dari row
                USINT8 *PX = row + x * 3;
                USINT8 B = PX[0];
                USINT8 G = PX[1];
                USINT8 R = PX[2];

                // susun jadi ARGB (0xAARRGGBB)
                USINT32 color = (0xFF << 24)    // alpha opaque
                            | (R    << 16)
                            | (G    <<  8)
                            | (B    <<  0);

                put_pixel(screen_x, screen_y, color);
            }
        }
    }
    serial_printf("[BGRT] Done showing BGRT\n");

    return STATUS_OK;

}