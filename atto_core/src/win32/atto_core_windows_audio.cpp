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

        return true;
    }

    void WindowsCore::AudioUpdate() {
        fmodSystem->update();
    }

    void WindowsCore::AudioShudown() {

    }

    AudioResource * WindowsCore::ResourceGetAndLoadAudio( const char * name ) {
        const i32 audioResourceCount = resources.audios.GetCount();
        for( i32 audioIndex = 0; audioIndex < audioResourceCount; audioIndex++ ) {
            AudioResource & audioResource = resources.audios[ audioIndex ];
            if( audioResource.name == name ) {
                return &audioResource;
            }
        }

        Win32AudioResource audioResource = {};
        audioResource.name = name;
        FMOD_RESULT result = fmodSystem->createSound( name, FMOD_DEFAULT, 0, &audioResource.sound );
        ERRCHECK( result );

        return resources.audios.Add_MemCpyPtr( &audioResource );
    }

    AudioSpeaker WindowsCore::AudioPlay( AudioResource * audio, f32 volume, bool looping ) {
        Win32AudioResource * win32Audio = (Win32AudioResource *)audio;
        if( win32Audio->sound == nullptr ) {
            INVALID_CODE_PATH;
            return {};
        }

        FMOD::Channel * channel = nullptr;
        FMOD_RESULT result = fmodSystem->playSound( win32Audio->sound, 0, false, &channel );
        ERRCHECK( result );

        return {};
    }

    AudioSpeaker WindowsCore::AudioPlay( AudioResource * audioResource, glm::vec2 pos, glm::vec2 vel, f32 volume /*= 1.0f*/, bool looping /*= false */ ) {
        AudioPlay( audioResource );
        return {};
    }

    void WindowsCore::AudioSetListener( glm::vec2 pos, glm::vec2 vel ) {

    }

}