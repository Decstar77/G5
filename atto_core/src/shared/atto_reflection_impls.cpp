#include "atto_core.h"
#include "../game/modes/atto_game_mode_game.h"

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

        REFLECT_STRUCT_BEGIN( SpriteActuation )
        REFLECT_STRUCT_MEMBER( frameIndex )
        REFLECT_STRUCT_MEMBER( audioIds )
        REFLECT_STRUCT_END()

        REFLECT_STRUCT_BEGIN( SpriteResource )
        REFLECT_STRUCT_MEMBER( spriteId )
        REFLECT_STRUCT_MEMBER( frameCount )
        REFLECT_STRUCT_MEMBER( frameWidth )
        REFLECT_STRUCT_MEMBER( frameHeight )
        REFLECT_STRUCT_MEMBER( frameRate )
        REFLECT_STRUCT_MEMBER( origin )
        REFLECT_STRUCT_MEMBER( frameActuations )
        REFLECT_STRUCT_END()
}