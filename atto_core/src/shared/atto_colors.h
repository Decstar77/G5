#pragma once

#include "atto_math.h"

namespace atto {

    class Colors {
    public:
        static glm::vec4    FromHex( const char * h );
        static u32          VecToU32( const glm::vec4 & v );

    public:
        inline static glm::vec4 WHITE = glm::vec4( 1, 1, 1, 1 );
        inline static glm::vec4 BLACK = glm::vec4( 0, 0, 0, 1 );
        inline static glm::vec4 RED = glm::vec4( 0.9f, 0, 0, 1 );
        inline static glm::vec4 GREEN = glm::vec4( 0, 0.9f, 0, 1 );
        inline static glm::vec4 BLUE = glm::vec4( 0, 0, 0.9f, 1 );
        inline static glm::vec4 YELLOW = glm::vec4( 0.9f, 0.9f, 0, 1 );
        inline static glm::vec4 CYAN = glm::vec4( 0, 0.9f, 0.9f, 1 );
        inline static glm::vec4 MAGENTA = glm::vec4( 0.9f, 0, 0.9f, 1 );
        inline static glm::vec4 SKY_BLUE = glm::vec4( 0.5f, 0.5f, 1, 1 );
        inline static glm::vec4 PETER_RIVER = FromHex( "#3498db" );
        inline static glm::vec4 AMETHYST = FromHex( "#9b59b6" );
        inline static glm::vec4 MIDNIGHT_BLUE = FromHex( "#2c3e50" );
        inline static glm::vec4 SUN_FLOWER = FromHex( "#f1c40f" );
        inline static glm::vec4 CARROT = FromHex( "#e67e22" );
        inline static glm::vec4 ALIZARIN = FromHex( "#e74c3c" );
        inline static glm::vec4 SILVER = FromHex( "#bdc3c7" );



        // Game specific colors
        inline static glm::vec4 BOX_SELECTION_COLOR = glm::vec4( 0.5f, 1, 0.5f, 0.5f );
    };

}
