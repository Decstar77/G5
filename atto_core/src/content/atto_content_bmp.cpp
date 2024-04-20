#include "atto_content_bmp.h"

#include <fstream>
#include <iostream>

namespace atto {
    static void WriteHeaders( BMP * bmp, std::ofstream & of ) {
        of.write( ( const char *) & bmp->fileHeader, sizeof( bmp->fileHeader ) );
        of.write( ( const char *) & bmp->bmpInfoHeader, sizeof( bmp->bmpInfoHeader ) );
        if ( bmp->bmpInfoHeader.bit_count == 32 ) {
            of.write( ( const char *) & bmp->bmpColorHeader, sizeof( bmp->bmpColorHeader ) );
        }
    }

    static void WriteHeadersAndData( BMP * bmp, std::ofstream & of ) {
        WriteHeaders( bmp, of );
        of.write( ( const char * )bmp->data.data(), bmp->data.size() );
    }

    BMP::BMP( const char * fname ) {
        ReadFromFile( fname );
    }

    BMP::BMP( i32 width, i32 height, bool has_alpha /*= true */ ) {
        if( width <= 0 || height <= 0 ) {
            throw std::runtime_error( "The image width and height must be positive numbers." );
        }

        bmpInfoHeader.width = width;
        bmpInfoHeader.height = height;
        if( has_alpha ) {
            bmpInfoHeader.size = sizeof( BMPInfoHeader ) + sizeof( BMPColorHeader );
            fileHeader.offset_data = sizeof( BMPFileHeader ) + sizeof( BMPInfoHeader ) + sizeof( BMPColorHeader );

            bmpInfoHeader.bit_count = 32;
            bmpInfoHeader.compression = 3;
            rowStride = width * 4;
            data.resize( rowStride * height );
            fileHeader.file_size = fileHeader.offset_data + (u32)data.size();
        }
        else {
            bmpInfoHeader.size = sizeof( BMPInfoHeader );
            fileHeader.offset_data = sizeof( BMPFileHeader ) + sizeof( BMPInfoHeader );

            bmpInfoHeader.bit_count = 24;
            bmpInfoHeader.compression = 0;
            rowStride = width * 3;
            data.resize( rowStride * height );

            u32 new_stride = MakeStrideAligend( 4 );
            fileHeader.file_size = fileHeader.offset_data + static_cast<u32>( data.size() ) + bmpInfoHeader.height * ( new_stride - rowStride );
        }
    }

    void BMP::WriteToFile( const char * fname ) {
        std::ofstream of{ fname, std::ios_base::binary };
        if( of ) {
            if( bmpInfoHeader.bit_count == 32 ) {
                WriteHeadersAndData( this, of );
            }
            else if( bmpInfoHeader.bit_count == 24 ) {
                if( bmpInfoHeader.width % 4 == 0 ) {
                    WriteHeadersAndData( this, of );
                }
                else {
                    u32 new_stride = MakeStrideAligend( 4 );
                    std::vector<byte> padding_row( new_stride - rowStride );

                    WriteHeaders( this, of );

                    for( int y = 0; y < bmpInfoHeader.height; ++y ) {
                        of.write( (const char *)( data.data() + rowStride * y ), rowStride );
                        of.write( (const char *)padding_row.data(), padding_row.size() );
                    }
                }
            }
            else {
                throw std::runtime_error( "The program can treat only 24 or 32 bits per pixel BMP files" );
            }
        }
        else {
            throw std::runtime_error( "Unable to open the output image file." );
        }
    }

    void BMP::FillRegion( u32 x0, u32 y0, u32 w, u32 h, byte R, byte G, byte B, byte A ) {
        if( x0 + w > (u32)bmpInfoHeader.width || y0 + h > (u32)bmpInfoHeader.height ) {
            throw std::runtime_error( "The region does not fit in the image!" );
        }

        u32 channels = bmpInfoHeader.bit_count / 8;
        for( u32 y = y0; y < y0 + h; ++y ) {
            for( u32 x = x0; x < x0 + w; ++x ) {
                data[ channels * ( y * bmpInfoHeader.width + x ) + 0 ] = B;
                data[ channels * ( y * bmpInfoHeader.width + x ) + 1 ] = G;
                data[ channels * ( y * bmpInfoHeader.width + x ) + 2 ] = R;
                if( channels == 4 ) {
                    data[ channels * ( y * bmpInfoHeader.width + x ) + 3 ] = A;
                }
            }
        }
    }

    void BMP::SetPixel( u32 x0, u32 y0, byte R, byte G, byte B, byte A ) {
        if( x0 >= (u32)bmpInfoHeader.width || y0 >= (u32)bmpInfoHeader.height || x0 < 0 || y0 < 0 ) {
            throw std::runtime_error( "The point is outside the image boundaries!" );
        }

        u32 channels = bmpInfoHeader.bit_count / 8;
        data[ channels * ( y0 * bmpInfoHeader.width + x0 ) + 0 ] = B;
        data[ channels * ( y0 * bmpInfoHeader.width + x0 ) + 1 ] = G;
        data[ channels * ( y0 * bmpInfoHeader.width + x0 ) + 2 ] = R;
        if( channels == 4 ) {
            data[ channels * ( y0 * bmpInfoHeader.width + x0 ) + 3 ] = A;
        }
    }

    void BMP::DrawRect( u32 x0, u32 y0, u32 w, u32 h, byte R, byte G, byte B, byte A, byte line_w ) {
        if( x0 + w > (u32)bmpInfoHeader.width || y0 + h > (u32)bmpInfoHeader.height ) {
            throw std::runtime_error( "The rectangle does not fit in the image!" );
        }

        FillRegion( x0, y0, w, line_w, B, G, R, A );                                             // top line
        FillRegion( x0, ( y0 + h - line_w ), w, line_w, B, G, R, A );                              // bottom line
        FillRegion( ( x0 + w - line_w ), ( y0 + line_w ), line_w, ( h - ( 2 * line_w ) ), B, G, R, A );  // right line
        FillRegion( x0, ( y0 + line_w ), line_w, ( h - ( 2 * line_w ) ), B, G, R, A );                 // left line
    }

    void BMP::FlipVertically() {
        u32 num_rows = bmpInfoHeader.height;
        u32 bytes_per_row = bmpInfoHeader.width * ( bmpInfoHeader.bit_count / 8 );
        std::vector<byte> temp_row( bytes_per_row );
        for( u32 row = 0; row < num_rows / 2; ++row ) {
            u32 current_row_index = row * bytes_per_row;
            u32 opposite_row_index = ( num_rows - row - 1 ) * bytes_per_row;
            std::copy( data.begin() + current_row_index, data.begin() + current_row_index + bytes_per_row, temp_row.begin() );
            std::copy( data.begin() + opposite_row_index, data.begin() + opposite_row_index + bytes_per_row, data.begin() + current_row_index );
            std::copy( temp_row.begin(), temp_row.end(), data.begin() + opposite_row_index );
        }
    }

    u32 BMP::MakeStrideAligend( u32 align_stride ) {
        u32 new_stride = rowStride;
        while( new_stride % align_stride != 0 ) {
            new_stride++;
        }
        return new_stride;
    }

    void BMP::CheckColorHeader( BMPColorHeader & bmp_color_header ) {
        BMPColorHeader expected_color_header;
        if( expected_color_header.red_mask != bmp_color_header.red_mask ||
            expected_color_header.blue_mask != bmp_color_header.blue_mask ||
            expected_color_header.green_mask != bmp_color_header.green_mask ||
            expected_color_header.alpha_mask != bmp_color_header.alpha_mask ) {
            throw std::runtime_error( "Unexpected color mask format! The program expects the pixel data to be in the BGRA format" );
        }
        if( expected_color_header.color_space_type != bmp_color_header.color_space_type ) {
            throw std::runtime_error( "Unexpected color space type! The program expects sRGB values" );
        }
    }

    void BMP::ReadFromFile( const char * fname ) {
        std::ifstream inp{ fname, std::ios_base::binary };
        if( inp ) {
            inp.read( (char *)&fileHeader, sizeof( fileHeader ) );
            if( fileHeader.file_type != 0x4D42 ) {
                throw std::runtime_error( "Error! Unrecognized file format." );
            }
            inp.read( (char *)&bmpInfoHeader, sizeof( bmpInfoHeader ) );

            // The BMPColorHeader is used only for transparent images
            if( bmpInfoHeader.bit_count == 32 ) {
                // Check if the file has bit mask color information
                if( bmpInfoHeader.size >= ( sizeof( BMPInfoHeader ) + sizeof( BMPColorHeader ) ) ) {
                    inp.read( (char *)&bmpColorHeader, sizeof( bmpColorHeader ) );
                    // Check if the pixel data is stored as BGRA and if the color space type is sRGB
                    CheckColorHeader( bmpColorHeader );
                }
                else {
                    std::cerr << "Error! The file \"" << fname << "\" does not seem to contain bit mask information\n";
                    throw std::runtime_error( "Error! Unrecognized file format." );
                }
            }

            // Jump to the pixel data location
            inp.seekg( fileHeader.offset_data, inp.beg );

            // Adjust the header fields for output.
            // Some editors will put extra info in the image file, we only save the headers and the data.
            if( bmpInfoHeader.bit_count == 32 ) {
                bmpInfoHeader.size = sizeof( BMPInfoHeader ) + sizeof( BMPColorHeader );
                fileHeader.offset_data = sizeof( BMPFileHeader ) + sizeof( BMPInfoHeader ) + sizeof( BMPColorHeader );
            }
            else {
                bmpInfoHeader.size = sizeof( BMPInfoHeader );
                fileHeader.offset_data = sizeof( BMPFileHeader ) + sizeof( BMPInfoHeader );
            }
            fileHeader.file_size = fileHeader.offset_data;

            if( bmpInfoHeader.height < 0 ) {
                throw std::runtime_error( "The program can treat only BMP images with the origin in the bottom left corner!" );
            }

            data.resize( bmpInfoHeader.width * bmpInfoHeader.height * bmpInfoHeader.bit_count / 8 );

            // Here we check if we need to take into account row padding
            if( bmpInfoHeader.width % 4 == 0 ) {
                inp.read( (char *)data.data(), data.size() );
                fileHeader.file_size += static_cast<u32>( data.size() );
            }
            else {
                rowStride = bmpInfoHeader.width * bmpInfoHeader.bit_count / 8;
                u32 new_stride = MakeStrideAligend( 4 );
                std::vector<byte> padding_row( new_stride - rowStride );

                for( int y = 0; y < bmpInfoHeader.height; ++y ) {
                    inp.read( (char *)( data.data() + rowStride * y ), rowStride );
                    inp.read( (char *)padding_row.data(), padding_row.size() );
                }
                fileHeader.file_size += static_cast<u32>( data.size() ) + bmpInfoHeader.height * static_cast<u32>( padding_row.size() );
            }
        }
        else {
            throw std::runtime_error( "Unable to open the input image file." );
        }
    }
}
