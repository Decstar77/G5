#pragma once

#include "../shared/atto_defines.h"
#include "../shared/atto_containers.h"

#include <vector>

namespace atto {
     // https://github.com/sol-prog/cpp-bmp-images
#pragma pack(push, 1)
    struct BMPFileHeader {
        u16 file_type = 0x4D42;
        u32 file_size = 0;
        u16 reserved1 = 0;
        u16 reserved2 = 0;
        u32 offset_data = 0;
    };

    struct BMPInfoHeader {
        u32 size = 0;
        i32 width = 0;
        i32 height = 0;
        u16 planes = 1;
        u16 bit_count = 0;
        u32 compression = 0;
        u32 size_image = 0;
        i32 x_pixels_per_meter = 0;
        i32 y_pixels_per_meter = 0;
        u32 colors_used = 0;
        u32 colors_important = 0;
    };

    struct BMPColorHeader {
        u32 red_mask = 0x00ff0000;              // Bit mask for the red channel
        u32 green_mask = 0x0000ff00;            // Bit mask for the green channel
        u32 blue_mask = 0x000000ff;             // Bit mask for the blue channel
        u32 alpha_mask = 0xff000000;            // Bit mask for the alpha channel
        u32 color_space_type = 0x73524742;      // Default "sRGB" (0x73524742)
        u32 unused[ 16 ] = {};                  // Unused data for sRGB color space
    };

#pragma pack(pop)

    class BMP {
    public:
        BMP( const char * fname );
        BMP( i32 width, i32 height, bool has_alpha = true );

        void ReadFromFile( const char * fname );
        void WriteToFile( const char * fname );
        void FillRegion( u32 x0, u32 y0, u32 w, u32 h, byte R, byte G, byte B, byte A );
        void SetPixel( u32 x0, u32 y0, byte R, byte G, byte B, byte A );
        void DrawRect( u32 x0, u32 y0, u32 w, u32 h, byte R, byte G, byte B, byte A, byte line_w );
        void FlipVertically();

    public:
        BMPFileHeader fileHeader;
        BMPInfoHeader bmpInfoHeader;
        BMPColorHeader bmpColorHeader;
        std::vector<byte> data;

    private:
        u32     rowStride = 0;
        u32     MakeStrideAligend( u32 align_stride );
        void    CheckColorHeader( BMPColorHeader & bmp_color_header );
    };
}
