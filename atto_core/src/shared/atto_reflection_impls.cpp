#include "atto_core.h"

namespace atto {
    REFLECT_STRUCT_BEGIN( GameSettings )
    REFLECT_STRUCT_MEMBER( windowWidth )
    REFLECT_STRUCT_MEMBER( windowHeight )
    REFLECT_STRUCT_MEMBER( windowStartPosX )
    REFLECT_STRUCT_MEMBER( windowStartPosY )
    REFLECT_STRUCT_MEMBER( noAudio )
    REFLECT_STRUCT_MEMBER( fullscreen )
    REFLECT_STRUCT_MEMBER( vsync )
    REFLECT_STRUCT_MEMBER( showDebug )
    REFLECT_STRUCT_MEMBER( basePath )
    REFLECT_STRUCT_END()
}