#include "atto_content.h"

#include <vector> // @NOTE(DECLAN): Going to use std vector because none of this code should run in release

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image/std_image.h>

namespace atto {

    ContentTextureProcesor::ContentTextureProcesor() {

    }

    ContentTextureProcesor::~ContentTextureProcesor() {
        if( pixelData != nullptr ) {
            stbi_image_free( pixelData );
        }
    }

    bool ContentTextureProcesor::LoadFromFile( const char * file ) {
        LargeString filePath = StringFormat::Large( "res/sprites/%s", file );
        pixelData = stbi_load( filePath.GetCStr(), &width, &height, &channels, 4 );

        if( pixelData == nullptr ) {
            //LogOutput( LogLevel::ERR, "Failed to load texture asset \t %s", name );
            return false;
        }
        

        return true;
    }

    void ContentTextureProcesor::FixAplhaEdges() {
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

}