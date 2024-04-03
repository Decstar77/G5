#include "atto_content.h"

#include <vector> // @NOTE(DECLAN): Going to use std vector because none of this code should run in release

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image/std_image.h>

namespace atto {

    inline void static GetPixel( byte * pixelData, i32 width, i32 height, i32 x, i32 y, u8 & r, u8 & g, u8 & b, u8 & a ) {
        Assert( x < width && x >= 0 );
        Assert( y < height && y >= 0 );

        i32 offset = ( y * width + x ) * 4;
        r = pixelData[ offset + 0 ];
        g = pixelData[ offset + 1 ];
        b = pixelData[ offset + 2 ];
        a = pixelData[ offset + 3 ];
    }

    ContentTextureProcessor::ContentTextureProcessor() {

    }

    ContentTextureProcessor::~ContentTextureProcessor() {
        if( pixelData != nullptr ) {
            free( pixelData );
        }
    }

    bool ContentTextureProcessor::LoadFromFile( const char * file ) {
        filePath = file;
        pixelData = stbi_load( filePath.GetCStr(), &width, &height, &channels, 4 );
        channels = 4;

        if( pixelData == nullptr ) {
            return false;
        }

        return true;
    }


    void ContentTextureProcessor::MakeAlphaEdge() {
        if( channels != 4 ) {
            return;
        }

        bool needsBorder = false;
        for( int y = 0; y < height; y++ ) {
            for( int x = 0; x < width; x++ ) {
                u8 r;
                u8 g;
                u8 b;
                u8 a;
                GetPixel( pixelData, width, height, x, y, r, g, b, a );

                if( a != 0 ) {
                    needsBorder = true;
                }
            }
        }

        if( needsBorder == false ) {
            return;
        }

        const i32 newWidth = width + 2;
        const i32 newHeight = height + 2;
        const i32 newSize = newWidth * newHeight * 4;
        byte * newData = (byte *)malloc( newSize );

        if( newData == nullptr ) {
            INVALID_CODE_PATH;
            return;
        }

        memset( newData, 0, newSize );

        for( int y = 1; y < newHeight - 1; y++ ) {
            for( int x = 1; x < newWidth - 1; x++ ) {
                u8 r;
                u8 g;
                u8 b;
                u8 a;
                GetPixel( pixelData, width, height, x - 1, y - 1, r, g, b, a );

                newData[ ( y * newWidth + x ) * 4 + 0 ] = r;
                newData[ ( y * newWidth + x ) * 4 + 1 ] = g;
                newData[ ( y * newWidth + x ) * 4 + 2 ] = b;
                newData[ ( y * newWidth + x ) * 4 + 3 ] = a;
            }
        }

        free( pixelData );

        pixelData = newData;
        width = newWidth;
        height = newHeight;
    }

    void ContentTextureProcessor::FixAplhaEdges() {
        const int sizeBytes = width * height * 4;

        // copy data into vec
        std::vector<byte> dcopy( sizeBytes );
        memcpy( dcopy.data(), pixelData, sizeBytes );

        const byte * srcptr = dcopy.data();
        byte * data_ptr = pixelData;

        const int max_radius = 4;
        const int alpha_threshold = 20;
        const int max_dist = 0x7FFFFFFF;

        for( int i = 0; i < height; i++ ) {
            for( int j = 0; j < width; j++ ) {
                const byte * rptr = &srcptr[ ( i * width + j ) * 4 ];
                byte * wptr = &data_ptr[ ( i * width + j ) * 4 ];

                if( rptr[ 3 ] >= alpha_threshold ) {
                    continue;
                }

                int closest_dist = max_dist;
                byte closest_color[ 3 ] = { 0 };

                int from_x = Max( 0, j - max_radius );
                int to_x = Min( width - 1, j + max_radius );
                int from_y = Max( 0, i - max_radius );
                int to_y = Min( height - 1, i + max_radius );

                for( int k = from_y; k <= to_y; k++ ) {
                    for( int l = from_x; l <= to_x; l++ ) {
                        int dy = i - k;
                        int dx = j - l;
                        int dist = dy * dy + dx * dx;
                        if( dist >= closest_dist ) {
                            continue;
                        }

                        const byte * rp2 = &srcptr[ ( k * width + l ) << 2 ];

                        if( rp2[ 3 ] < alpha_threshold ) {
                            continue;
                        }

                        closest_dist = dist;
                        closest_color[ 0 ] = rp2[ 0 ];
                        closest_color[ 1 ] = rp2[ 1 ];
                        closest_color[ 2 ] = rp2[ 2 ];
                    }
                }

                if( closest_dist != max_dist ) {
                    wptr[ 0 ] = closest_color[ 0 ];
                    wptr[ 1 ] = closest_color[ 1 ];
                    wptr[ 2 ] = closest_color[ 2 ];
                }
            }
        }
    }


    bool PixelArtMeshCreator::LoadFromFile( const char * file ) {
        i32 width = 0;
        i32 height = 0;
        i32 channels = 0;

        stbi_set_flip_vertically_on_load( true );
        byte * pixelData = stbi_load( file, &width, &height, &channels, 4 );
        stbi_set_flip_vertically_on_load( false );

        if( pixelData != nullptr ) {
            for ( i32 y = 0; y < height; y++ ) {
                for ( i32 x = 0; x < width; x++ ) {
                    u8 r;
                    u8 g;
                    u8 b;
                    u8 a;
                    GetPixel( pixelData, width, height, x, y, r, g, b, a );

                    if ( a != 0 ) {
                        f32 rr = ( ( f32 )r ) / 255.0f;
                        f32 gg = ( ( f32 )g ) / 255.0f;
                        f32 bb = ( ( f32 )b ) / 255.0f;
                        f32 aa = ( ( f32 )a ) / 255.0f;

                        f32 pixelSize = 10.0f;

                        PixyPoint pp = {};
                        pp.c = glm::vec4( rr, gg, bb, aa );
                        pp.p = ( glm::vec2( x, y ) - glm::vec2( width, height ) / 2.0f + glm::vec2( 0.5f ) ) * pixelSize;

                        points.push_back( pp );
                    }
                }
            }

            free( pixelData );
            return true;
        }
        return false;
    }
}