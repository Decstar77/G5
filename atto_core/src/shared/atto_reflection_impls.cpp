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
        REFLECT_STRUCT_MEMBER( usePackedAssets )
        REFLECT_STRUCT_MEMBER( basePath )
        REFLECT_STRUCT_MEMBER( masterVolume )
        REFLECT_STRUCT_END()

        REFLECT_STRUCT_BEGIN( TextureResource )
        REFLECT_STRUCT_MEMBER( id )
        REFLECT_STRUCT_MEMBER( name )
        REFLECT_STRUCT_MEMBER( hasAnti )
        REFLECT_STRUCT_MEMBER( hasMips )
        REFLECT_STRUCT_END()

        REFLECT_STRUCT_BEGIN( AudioResource )
        REFLECT_STRUCT_MEMBER( id )
        REFLECT_STRUCT_MEMBER( name )
        REFLECT_STRUCT_MEMBER( is2D )
        REFLECT_STRUCT_MEMBER( is3D )
        REFLECT_STRUCT_MEMBER( minDist )
        REFLECT_STRUCT_MEMBER( maxDist )
        REFLECT_STRUCT_END()

        REFLECT_STRUCT_BEGIN( SpriteActuation )
        REFLECT_STRUCT_MEMBER( frameIndex )
        REFLECT_STRUCT_MEMBER( audioResources )
        REFLECT_STRUCT_END()

        REFLECT_STRUCT_BEGIN( SpriteResource )
        REFLECT_STRUCT_MEMBER( spriteId )
        REFLECT_STRUCT_MEMBER( textureResource )
        REFLECT_STRUCT_MEMBER( isTileMap )
        REFLECT_STRUCT_MEMBER( tileWidth )
        REFLECT_STRUCT_MEMBER( tileHeight )
        REFLECT_STRUCT_MEMBER( frameCount )
        REFLECT_STRUCT_MEMBER( frameWidth )
        REFLECT_STRUCT_MEMBER( frameHeight )
        REFLECT_STRUCT_MEMBER( frameRate )
        REFLECT_STRUCT_MEMBER( origin )
        REFLECT_STRUCT_MEMBER( frameActuations )
        REFLECT_STRUCT_END()

        REFLECT_STRUCT_BEGIN( BoxBounds2D )
        REFLECT_STRUCT_MEMBER( min )
        REFLECT_STRUCT_MEMBER( max )
        REFLECT_STRUCT_END()

}