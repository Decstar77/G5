#pragma once

#include "atto_defines.h"
#include "atto_containers.h"
#include "atto_logging.h"
#include "atto_reflection.h"

namespace FMOD {
    class System;
    class Sound;
    class Channel;
    class ChannelGroup;
}

namespace atto {
    class Resource {
    public:
        u32             id;
        LargeString     name;

        LargeString     GetShortName() const;
    };

    struct TextureResourceCreateInfo {
        bool hasMips;
        bool hasAnti;
    };

    class TextureResource : public Resource {
    public:
        u64     handle;
        i32     width;
        i32     height;
        i32     channels;
        TextureResourceCreateInfo createInfo;

        inline i32 GetByteSize() const { i32 size = width * height * channels * 1; return size; }

        REFLECT();
    };

    struct AudioResourceCreateInfo {
        bool is2D;
        bool is3D;
        f32 minDist;
        f32 maxDist;
    };

    enum class AudioStealMode {
        NONE = 0,
        OLDEST = 1,
    };

    class AudioResource : public Resource {
    public:
        FMOD::Sound *   sound2D;
        FMOD::Sound *   sound3D;
        f32             volumeMultiplier;
        AudioResourceCreateInfo createInfo;
        REFLECT();
    };
    
    struct AudioGroupResourceCreateInfo {
        i32                             maxInstances;
        f64                             minTimeToPassForAnotherSubmission; // @NOTE: In seconds
        AudioStealMode                  stealMode;
        FixedList<AudioResource *, 8>   sounds;
    };

    class AudioGroupResource : public Resource {
    public:
        i32                             maxInstances;
        f64                             minTimeToPassForAnotherSubmission; // @NOTE: In seconds
        AudioStealMode                  stealMode;
        FixedList<AudioResource *, 8>   sounds;

        REFLECT();
    };

    struct SpriteActuation {
        i32                              frameIndex;
        FixedList< AudioResource *, 4 >  audioResources;

        REFLECT();
    };

    struct SpriteResourceCreateInfo {
        i32 frameCount;
        i32 frameXCount;
        i32 frameYCount;
        i32 frameWidth;
        i32 frameHeight;
        i32 frameRate;
        bool bakeInAtlas;

        REFLECT();
    };

    class SpriteResource : public Resource {
    public:
        TextureResource *               textureResource;
        SpriteResourceCreateInfo        createInfo;
        FixedList< SpriteActuation, 4 > frameActuations;

        void                            GetUVForTile( i32 tileX, i32 tileY, glm::vec2 & bl, glm::vec2 & tr ) const;

        REFLECT();
    };

    TextureResource *                   ResourceGetAndCreateTexture( const char * name, TextureResourceCreateInfo createInfo = {} );
    TextureResource *                   ResourceGetAndLoadTexture( const char * name );

    AudioResource *                     ResourceGetAndCreateAudio( const char * name, AudioResourceCreateInfo createInfo = {} );
    AudioResource *                     ResourceGetAndCreateAudio2D( const char * name );
    AudioResource *                     ResourceGetAndLoadAudio( const char * name );

    AudioGroupResource *                ResourceGetAndCreateAudioGroup( const char * name, AudioGroupResourceCreateInfo * createInfo );
    AudioGroupResource *                ResourceGetAndLoadAudioGroup( const char * name );

    SpriteResource *                    ResourceGetAndCreateSprite( const char * spriteName, SpriteResourceCreateInfo createInfo = {} );
    SpriteResource *                    ResourceGetAndLoadSprite( const char * spriteName );
    void                                ResourceGetAndLoadSpriteCollection( Span<LargeString> spriteFiles );

    FontHandle                          ResourceGetFont( const char * name );

    char *                              ResourceReadEntireTextFileIntoPermanentMemory( const char * path, i64 * size );
    char *                              ResourceReadEntireTextFileIntoTransientMemory( const char * path, i64 * size );
    char *                              ResourceReadEntireBinaryFileIntoPermanentMemory( const char * path, i64 * size );
    char *                              ResourceReadEntireBinaryFileIntoTransientMemory( const char * path, i64 * size );

    template< typename _type_ >
    inline void ResourceWriteTextRefl( const _type_ * obj, const char * path ) {
        TypeDescriptor * settingsType = TypeResolver<_type_>::get();
        nlohmann::json j = settingsType->JSON_Write( obj );
        PlatformWriteEntireTextFile( path, j.dump().c_str() );
    }

    template< typename _type_>
    inline bool ResourceReadTextRefl( _type_ * obj, const char * path ) {
        i64 fileSize;
        char * data = ResourceReadEntireTextFileIntoTransientMemory( path, &fileSize );
        if( data != nullptr ) {
            nlohmann::json j = nlohmann::json::parse( data );
            TypeDescriptor * settingsType = TypeResolver<_type_>::get();
            settingsType->JSON_Read( j, obj );
            return true;
        }
        else {
            ATTOERROR( "ResourceReadTextRefl :: Could not load %s", path );
        }

        return false;
    }
}

