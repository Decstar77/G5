#include "atto_resources.h"

namespace atto {

    struct ResourceRegistry {
        FixedList<TextureResource, 1024>                textures;
        FixedList<AudioResource, 1024>                  audios;
        FixedList<AudioGroupResource, 1024>             audioGroups;
        FixedList<SpriteResource, 1024>                 sprites;
    };

    static ResourceRegistry resources = {};
    static LargeString texturePathMatch = {};
    static LargeString texturePathFill = {};
    // This function is for the replacing the "/blue/" part of paths. For instance to "/red/". This way we can store only json file
    void ResourceReplaceSpriteLoadPath( const char * match, const char * fill ) {
        texturePathMatch = LargeString::FromLiteral( match );
        texturePathFill = LargeString::FromLiteral( fill );
    }

    TextureResource * ResourceGetAndCreateTexture( const char * rawName, TextureResourceCreateInfo createInfo ) {
        LargeString name = LargeString::FromLiteral( rawName );
        if ( texturePathFill.GetLength() > 0 ) {
            name.Replace( texturePathMatch.GetCStr(), texturePathFill.GetCStr() );
        }

        const i32 textureResourceCount = resources.textures.GetCount();
        for( i32 textureResourceIndex = 0; textureResourceIndex < textureResourceCount; textureResourceIndex++ ) {
            TextureResource & textureResource = resources.textures[ textureResourceIndex ];
            if( textureResource.name == name ) {
                return &textureResource;
            }
        }

        TextureResource * texture = &resources.textures.AddEmpty();
        texture->createInfo = createInfo;
        texture->name = name;
        texture->id = StringHash::Hash( name.GetCStr() );

        PlatformRendererCreateTexture( texture );

        return texture;
    }
    
    TextureResource * ResourceGetAndLoadTexture( const char * name ) {
        return nullptr;
    }

    AudioResource * ResourceGetAndCreateAudio( const char * name, AudioResourceCreateInfo createInfo ) {
        const i32 audioResourceCount = resources.audios.GetCount();
        for( i32 audioIndex = 0; audioIndex < audioResourceCount; audioIndex++ ) {
            AudioResource & audioResource = resources.audios[ audioIndex ];
            if( audioResource.name == name ) {
                return &audioResource;
            }
        }

        AudioResource * audioResource = &resources.audios.AddEmpty();
        audioResource->id = StringHash::Hash( name );
        audioResource->name = name;
        audioResource->volumeMultiplier = 1.0f;
        audioResource->createInfo = createInfo;

        PlatformRendererCreateAudio( audioResource );

        return audioResource;
    }
    
    AudioResource * ResourceGetAndCreateAudio2D( const char * name ) {
        AudioResourceCreateInfo createInfo = {};
        createInfo.is2D = true;
        return ResourceGetAndCreateAudio( name, createInfo );
    }

    AudioResource * ResourceGetAndLoadAudio( const char * name ) {
        const i32 audioResourceCount = resources.audios.GetCount();
        for( i32 audioIndex = 0; audioIndex < audioResourceCount; audioIndex++ ) {
            AudioResource & audioResource = resources.audios[ audioIndex ];
            if( audioResource.name == name ) {
                return &audioResource;
            }
        }

        AudioResource * audioResource = &resources.audios.AddEmpty();
        audioResource->id = StringHash::Hash( name );
        audioResource->name = name;
        audioResource->volumeMultiplier = 1.0f;
        audioResource->createInfo.is2D = true;

        PlatformRendererCreateAudio( audioResource );

        return audioResource;
    }

    AudioGroupResource * ResourceGetAndCreateAudioGroup( const char * name, AudioGroupResourceCreateInfo * createInfo ) {
        const i32 audioResourceGroupCount = resources.audioGroups.GetCount();
        for( i32 groupIndex = 0; groupIndex < audioResourceGroupCount; groupIndex++ ) {
            AudioGroupResource & groupResource = resources.audioGroups[ groupIndex ];
            if( groupResource.name == name ) {
                return &groupResource;
            }
        }

        AudioGroupResource * groupResource = &resources.audioGroups.AddEmpty();
        groupResource->id = StringHash::Hash( name );
        groupResource->name = name;
        groupResource->maxInstances = 1;
        groupResource->maxInstances = createInfo->maxInstances;
        groupResource->stealMode = createInfo->stealMode;
        groupResource->minTimeToPassForAnotherSubmission = createInfo->minTimeToPassForAnotherSubmission;
        groupResource->sounds = createInfo->sounds;

        return groupResource;
    }

    AudioGroupResource * ResourceGetAndLoadAudioGroup( const char * name ) {
        const i32 audioResourceGroupCount = resources.audioGroups.GetCount();
        for( i32 groupIndex = 0; groupIndex < audioResourceGroupCount; groupIndex++ ) {
            AudioGroupResource & groupResource = resources.audioGroups[ groupIndex ];
            if( groupResource.name == name ) {
                return &groupResource;
            }
        }

        AudioGroupResource * groupResource = &resources.audioGroups.AddEmpty();
        groupResource->id = StringHash::Hash( name );
        groupResource->name = name;

        ResourceReadTextRefl( groupResource, name );

        return groupResource;
    }

    SpriteResource * ResourceGetAndCreateSprite( const char * spriteName, SpriteResourceCreateInfo createInfo ) {
        const i32 spriteResourceCount = resources.sprites.GetCount();
        for( i32 spriteIndex = 0; spriteIndex < spriteResourceCount; spriteIndex++ ) {
            SpriteResource & sprite = resources.sprites[ spriteIndex ];
            if( sprite.name == spriteName ) {
                return &sprite;
            }
        }

        SpriteResource * spriteResource = &resources.sprites.AddEmpty();
        spriteResource->id = StringHash::Hash( spriteName );
        spriteResource->name= spriteName;
        spriteResource->createInfo = createInfo;

        LargeString textureName = LargeString::FromLiteral( spriteName );
        textureName.StripFileExtension();
        textureName.Add( ".png" );
        spriteResource->textureResource = ResourceGetAndCreateTexture( textureName.GetCStr() );

        return resources.sprites.Add_MemCpyPtr( spriteResource );
    }

    SpriteResource * ResourceGetAndLoadSprite( const char * rawName ) {
        LargeString name = LargeString::FromLiteral( rawName );
        if ( texturePathFill.GetLength() > 0 ) {
            name.Replace( texturePathMatch.GetCStr(), texturePathFill.GetCStr() );
        }

        const i32 spriteResourceCount = resources.sprites.GetCount();
        for( i32 spriteIndex = 0; spriteIndex < spriteResourceCount; spriteIndex++ ) {
            SpriteResource & sprite = resources.sprites[ spriteIndex ];
            if( sprite.name == name ) {
                return &sprite;
            }
        }

        SpriteResource * spriteResource = &resources.sprites.AddEmpty();
        spriteResource->id = StringHash::Hash( name.GetCStr() );
        spriteResource->name = name;
        ResourceReadTextRefl( spriteResource, rawName );

        return spriteResource;
    }

    void ResourceGetAndLoadSpriteCollection( Span<LargeString> spriteFiles ) {
        const i32 spriteFileCount = spriteFiles.GetCount();
        for ( i32 spriteIndex = 0; spriteIndex < spriteFileCount; spriteIndex++ ) {
            LargeString spriteName = spriteFiles[spriteIndex];

            bool exists = false;
            const i32 spriteResourceCount = resources.sprites.GetCount();
            for( i32 spriteIndex = 0; spriteIndex < spriteResourceCount; spriteIndex++ ) {
                SpriteResource & sprite = resources.sprites[ spriteIndex ];
                if( sprite.name == spriteName ) {
                    exists = true;
                    break;
                }
            }

            if ( exists == true ) {
                ATTOWARN("ResourceGetAndLoadSpriteCollection -> Trying to load already loaded sprite: %s", spriteName.GetCStr() );
                continue;
            }

            ATTOTRACE( "Loading Sprite %s", spriteName.GetCStr() );
            SpriteResource * spriteResource = &resources.sprites.AddEmpty();
            spriteResource->id = StringHash::Hash( spriteName.GetCStr() );
            spriteResource->name= spriteName;
            ResourceReadTextRefl( spriteResource, spriteName.GetCStr() );
        }
    }

    FontHandle ResourceGetFont( const char * name ) {
        return {};
    }

    char * ResourceReadEntireTextFileIntoPermanentMemory( const char * path, i64 * size ) {
        *size = PlatformGetFileSize( path );
        if( *size == -1 ) {
            return nullptr;
        }

        char * data = (char *)MemoryAllocatePermanent( *size );
        if( data == nullptr ) {
            return nullptr;
        }

        PlatformReadEntireTextFile( path, data, (i32)( *size ) );

        return data;
    }

    char * ResourceReadEntireTextFileIntoTransientMemory( const char * path, i64 * size ) {
        *size = PlatformGetFileSize( path );
        if( *size == -1 ) {
            return nullptr;
        }

        char * data = (char *)MemoryAllocateTransient( *size );
        if( data == nullptr ) {
            return nullptr;
        }

        PlatformReadEntireTextFile( path, data, (i32)( *size ) );

        return data;
    }
    
    char * ResourceReadEntireBinaryFileIntoPermanentMemory( const char * path, i64 * size ) {
        *size = PlatformGetFileSize( path );
        if( *size == -1 ) {
            return nullptr;
        }

        char * data = (char *)MemoryAllocatePermanent( *size );
        if( data == nullptr ) {
            return nullptr;
        }

        PlatformReadEntireBinaryFile( path, data, (i32)( *size ) );

        return data;
    }

    char * ResourceReadEntireBinaryFileIntoTransientMemory( const char * path, i64 * size ) {
        *size = PlatformGetFileSize( path );
        if( *size == -1 ) {
            return nullptr;
        }

        char * data = (char *)MemoryAllocateTransient( *size );
        if( data == nullptr ) {
            return nullptr;
        }

        PlatformReadEntireBinaryFile( path, data, (i32)( *size ) );

        return data;
    }
}
