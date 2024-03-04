#include "atto_core_windows.h"

#include <fmod.hpp>
#define ERRCHECK(_result) if (result != FMOD_OK) { INVALID_CODE_PATH; }

namespace atto {
    bool WindowsCore::AudioInitialize() {
        FMOD_RESULT result = {};

        result = FMOD::System_Create( &fmodSystem );
        ERRCHECK( result );

        result = fmodSystem->init( 32, FMOD_INIT_NORMAL, nullptr );
        ERRCHECK( result );

        result = fmodSystem->createChannelGroup( "Master", &fmodMasterGroup );
        ERRCHECK( result );

        fmodMasterGroup->setVolume( theGameSettings.masterVolume );

        fmodSystem->set3DSettings( 1.0f, 1.0f, 1.0f );

        return true;
    }

    void WindowsCore::AudioUpdate() {
        fmodSystem->update();
    }

    void WindowsCore::AudioShudown() {

    }

    atto::AudioResource * WindowsCore::ResourceGetAndCreateAudio( const char * name, bool is2D, bool is3D, f32 minDist, f32 maxDist ) {
        const i32 audioResourceCount = resources.audios.GetCount();
        for( i32 audioIndex = 0; audioIndex < audioResourceCount; audioIndex++ ) {
            AudioResource & audioResource = resources.audios[ audioIndex ];
            if( audioResource.name == name ) {
                return &audioResource;
            }
        }

        Win32AudioResource audioResource = {};
        audioResource.id = StringHash::Hash( name );
        audioResource.name = name;
        audioResource.is2D = is2D;
        audioResource.is3D = is3D;
        audioResource.minDist = minDist;
        audioResource.maxDist = maxDist;

        if( audioResource.is2D == true ) {
            FMOD_RESULT result = fmodSystem->createSound( name, FMOD_DEFAULT, 0, &audioResource.sound2D );
            ERRCHECK( result );
        }

        if( audioResource.is3D == true ) {
            FMOD_RESULT result = fmodSystem->createSound( name, FMOD_3D, 0, &audioResource.sound3D );
            audioResource.sound3D->set3DMinMaxDistance( minDist, maxDist );
            ERRCHECK( result );
        }

        return resources.audios.Add_MemCpyPtr( &audioResource );
    }

    AudioSpeaker WindowsCore::AudioPlay( AudioResource * audio, glm::vec2 * pos ) {
        Win32AudioResource * win32Audio = (Win32AudioResource *)audio;

        if( pos == nullptr ) {
            if( win32Audio->sound2D != nullptr ) {
                FMOD::Channel * channel = nullptr;
                FMOD_RESULT result = fmodSystem->playSound( win32Audio->sound2D, fmodMasterGroup, false, &channel );
                ERRCHECK( result );
            }
            else {
                LogOutput( LogLevel::ERR, "WindowsCore::AudioPlay :: Tried to play sound but was null %s", audio->name.GetCStr() );
            }
        }
        else {
            if( win32Audio->sound3D != nullptr ) {
                FMOD::Channel * channel = nullptr;
                FMOD_RESULT result = fmodSystem->playSound( win32Audio->sound3D, fmodMasterGroup, false, &channel );
                ERRCHECK( result );
                FMOD_VECTOR v = {};
                v.x = pos->x;
                v.y = pos->y;
                channel->set3DAttributes( &v, nullptr );

                f32 dist = glm::distance( *pos, listenerPos );
                LogOutput( LogLevel::INFO, "Dist = %f", dist );
            }
            else {
                LogOutput( LogLevel::ERR, "WindowsCore::AudioPlay :: Tried to play sound but was null %s", audio->name.GetCStr() );
            }
        }

        return {};
    }

    void WindowsCore::AudioSetListener( glm::vec2 pos ) {
        listenerPos = pos;
        FMOD_VECTOR v = {};
        v.x = pos.x;
        v.y = pos.y;
        FMOD_RESULT result = fmodSystem->set3DListenerAttributes( 0, &v, nullptr, nullptr, nullptr );
        ERRCHECK( result );
    }

}