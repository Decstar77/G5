#include "atto_core_windows.h"

#include <fmod.hpp>
#define ERRCHECK(_result) if (result != FMOD_OK) { INVALID_CODE_PATH; }

namespace atto {
    bool AudioSpeaker::IsPlaying() {
        FMOD::Channel *channel = ( FMOD::Channel * ) fmodChannel;
        bool playing = false;
        if ( channel != nullptr ) {
            channel->isPlaying( &playing );
            if ( playing == false ) {
                fmodChannel = nullptr;
            }
        }
        return playing;
    }

    void AudioSpeaker::Stop() {
        FMOD::Channel *channel = ( FMOD::Channel * ) fmodChannel;
        if ( channel != nullptr ) {
            channel->stop();
            fmodChannel = nullptr;
        }
    }

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
        fmodActives.Clear();
        fmodSpeakers.GatherActiveObjs( fmodActives );
    }

    void WindowsCore::AudioShudown() {

    }

    AudioResource * WindowsCore::ResourceGetAndCreateAudio( const char * name, bool is2D, bool is3D, f32 minDist, f32 maxDist ) {
        const i32 audioResourceCount = resources.audios.GetCount();
        for( i32 audioIndex = 0; audioIndex < audioResourceCount; audioIndex++ ) {
            AudioResource & audioResource = resources.audios[ audioIndex ];
            if( audioResource.name == name ) {
                return &audioResource;
            }
        }

        FMODAudioResource audioResource = {};
        audioResource.id = StringHash::Hash( name );
        audioResource.name = name;
        audioResource.is2D = is2D;
        audioResource.is3D = is3D;
        audioResource.minDist = minDist;
        audioResource.maxDist = maxDist;
        audioResource.maxInstances = 1;
        audioResource.stealMode = AudioStealMode::NONE;

        if( theGameSettings.noAudio == false ) {
            if( audioResource.is2D == true ) {
                FMOD_RESULT result = fmodSystem->createSound( name, FMOD_DEFAULT, 0, &audioResource.sound2D );
                ERRCHECK( result );
            }

            if( audioResource.is3D == true ) {
                FMOD_RESULT result = fmodSystem->createSound( name, FMOD_3D, 0, &audioResource.sound3D );
                audioResource.sound3D->set3DMinMaxDistance( minDist, maxDist );
                ERRCHECK( result );
            }
        }

        return resources.audios.Add_MemCpyPtr( &audioResource );
    }

    AudioResource * WindowsCore::ResourceGetAndLoadAudio( const char * name ) {
        const i32 audioResourceCount = resources.audios.GetCount();
        for( i32 audioIndex = 0; audioIndex < audioResourceCount; audioIndex++ ) {
            AudioResource & audioResource = resources.audios[ audioIndex ];
            if( audioResource.name == name ) {
                return &audioResource;
            }
        }

        LargeString resPath = {};
        resPath.Add( name );
        resPath.StripFileExtension();
        resPath.Add( ".json" );

        AudioResource * resource = MemoryAllocateTransient<AudioResource>();
        if( ResourceReadTextRefl<AudioResource>( resource, resPath.GetCStr() ) == true ) {
            return ResourceGetAndCreateAudio( name, resource->is2D, resource->id, resource->minDist, resource->maxDist );
        }

        return nullptr;
    }

    FMOD_RESULT F_CALLBACK ChannelCallback( FMOD_CHANNELCONTROL * channelControl, FMOD_CHANNELCONTROL_TYPE controlType, FMOD_CHANNELCONTROL_CALLBACK_TYPE callbackType, void * commandData1, void * commandData2 ) {
        if( callbackType == FMOD_CHANNELCONTROL_CALLBACK_END ) {
            FMOD::Channel * channel = (FMOD::Channel *)channelControl;
            void * speakerData = nullptr;
            FMOD_RESULT result = channel->getUserData( &speakerData );
            ERRCHECK ( result );
            if ( speakerData != nullptr ) {
                AudioSpeaker * speaker = ( AudioSpeaker * ) speakerData;
                speaker->fmodChannel = nullptr;
                WindowsCore * core = ( WindowsCore * ) speaker->core;
                core->fmodSpeakers.Remove( speaker->handle );
            }
        }

        return FMOD_OK;
    }

    void WindowsCore::AudioPlay( AudioResource * audio, glm::vec2 * pos ) {
        if ( theGameSettings.noAudio == true ) {
            return;
        }

        FMODAudioResource * win32Audio = (FMODAudioResource *)audio;

        if( pos == nullptr ) {
            if( win32Audio->sound2D != nullptr ) {
                if ( audio->maxInstances > 0 ) {
                    i32 count = 0;
                    AudioSpeaker * oldest = nullptr;
                    const i32 activeCount = fmodActives.GetCount();
                    for ( i32 speakerIndex = 0; speakerIndex < activeCount; speakerIndex++ ) {
                        AudioSpeaker * speaker = fmodActives[speakerIndex];
                        if ( speaker->source == audio ) {
                            if ( oldest == nullptr || oldest->spawnTime > speaker->spawnTime ) {
                                oldest = speaker;
                            }
                            count++;
                        }
                    }

                    if ( count >= audio->maxInstances ) {
                        if ( audio->stealMode == AudioStealMode::NONE ) {
                            return;
                        } else if ( audio->stealMode == AudioStealMode::OLDEST ) {
                            oldest->Stop();
                        }
                    }
                }

                FMOD::Channel * channel = nullptr;
                FMOD_RESULT result = fmodSystem->playSound( win32Audio->sound2D, fmodMasterGroup, true, &channel );
                ERRCHECK( result );

                //channel->setVolume( 0.15f );
                channel->setVolume( 1.0f );
                channel->setCallback( ChannelCallback );

                if ( audio->name.Contains( "vespene" ) ) { // @HACK:
                    channel->setVolume( 2.0f );
                } else if ( audio->name.Contains("march") ) {
                    channel->setVolume( 0.5f );
                }
                
                AudioSpeakerHandle handle = {};
                AudioSpeaker * speaker = fmodSpeakers.Add( handle );
                if ( speaker != nullptr ) {
                    speaker->handle = handle;
                    speaker->source = audio;
                    speaker->fmodChannel = channel;
                    speaker->core = this;
                    speaker->spawnTime = GetTheCurrentTime();
                }
                channel->setUserData( speaker );
                channel->setPaused( false );
            }
            else {
                LogOutput( LogLevel::ERR, "WindowsCore::AudioPlay :: Tried to play sound but was null %s", audio->name.GetCStr() );
            }
        }
        else {
            if( win32Audio->sound3D != nullptr ) {
                FMOD::Channel * channel = nullptr;
                FMOD_RESULT result = fmodSystem->playSound( win32Audio->sound3D, fmodMasterGroup, true, &channel );
                ERRCHECK( result );
                FMOD_VECTOR v = {};
                v.x = pos->x;
                v.y = pos->y;
                v.z = 0.0f;
                channel->set3DAttributes( &v, nullptr );
                channel->setVolume( 0.3f );
                channel->setPaused( false );

                //f32 dist = glm::distance( *pos, listenerPos );
                //LogOutput( LogLevel::INFO, "Dist = %f", dist );
            }
            else {
                LogOutput( LogLevel::ERR, "WindowsCore::AudioPlay :: Tried to play sound but was null %s", audio->name.GetCStr() );
            }
        }
    }

    void WindowsCore::AudioSetListener( glm::vec2 pos ) {
        if ( theGameSettings.noAudio == true ) {
            return;
        }

        listenerPos = pos;
        FMOD_VECTOR v = {};
        v.x = pos.x;
        v.y = pos.y;
        v.z = 1.0f;
        FMOD_RESULT result = fmodSystem->set3DListenerAttributes( 0, &v, nullptr, nullptr, nullptr );
        ERRCHECK( result );
    }

}