#include "../game/atto_core.h"

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
        REFLECT_STRUCT_MEMBER( serverIp )
        REFLECT_STRUCT_END()

        REFLECT_STRUCT_BEGIN( TextureResource )
        REFLECT_STRUCT_MEMBER( id )
        REFLECT_STRUCT_MEMBER( name )
        REFLECT_STRUCT_END()

        REFLECT_STRUCT_BEGIN( AudioResource )
        REFLECT_STRUCT_MEMBER( id )
        REFLECT_STRUCT_MEMBER( name )
        REFLECT_STRUCT_END()

        REFLECT_STRUCT_BEGIN( SpriteActuation )
        REFLECT_STRUCT_MEMBER( frameIndex )
        REFLECT_STRUCT_MEMBER( audioResources )
        REFLECT_STRUCT_END()

        REFLECT_STRUCT_BEGIN( SpriteResourceCreateInfo )
        REFLECT_STRUCT_MEMBER( frameCount )
        REFLECT_STRUCT_MEMBER( frameXCount )
        REFLECT_STRUCT_MEMBER( frameYCount )
        REFLECT_STRUCT_MEMBER( frameWidth )
        REFLECT_STRUCT_MEMBER( frameHeight )
        REFLECT_STRUCT_MEMBER( frameRate )
        REFLECT_STRUCT_MEMBER( bakeInAtlas )
        REFLECT_STRUCT_END()

        REFLECT_STRUCT_BEGIN( SpriteResource )
        REFLECT_STRUCT_MEMBER( textureResource )
        REFLECT_STRUCT_MEMBER( createInfo )
        REFLECT_STRUCT_MEMBER( frameActuations )
        REFLECT_STRUCT_END()
}