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

        // @NOTE: Probably should have all game types in seperate file but eeehhh.
        REFLECT_STRUCT_BEGIN( MapFileBlock )
        REFLECT_STRUCT_MEMBER( xIndex )
        REFLECT_STRUCT_MEMBER( yIndex )
        REFLECT_STRUCT_END()

        REFLECT_STRUCT_BEGIN( MapFile )
        REFLECT_STRUCT_MEMBER( version )
        REFLECT_STRUCT_MEMBER( mapWidth )
        REFLECT_STRUCT_MEMBER( mapHeight )
        REFLECT_STRUCT_MEMBER( blocks )
        REFLECT_STRUCT_END()
}