#include "atto_win32_core.h"

#include <fmod.hpp>
#define ERRCHECK(_result) if (result != FMOD_OK) { INVALID_CODE_PATH; }

namespace atto {
    static FMOD::ChannelGroup *                fmodMasterGroup;
    static FMOD::System *                      fmodSystem;
    static FixedObjectPool<AudioSpeaker, 1024> fmodSpeakers;
    static FixedList<AudioSpeaker *, 1024>     fmodActives;

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
    
    void PlatformRendererCreateAudio( class AudioResource * audioResource ) {
        if( Core::theCore->theGameSettings.noAudio == false ) {
            if( audioResource->createInfo.is2D == true ) {
                //FMOD_RESULT result = fmodSystem->createSound( name, FMOD_DEFAULT | FMOD_NONBLOCKING, 0, &audioResource.sound2D );
                FMOD_RESULT result = fmodSystem->createSound( audioResource->name.GetCStr(), FMOD_DEFAULT, 0, &audioResource->sound2D );
                ERRCHECK( result );
            }

            if( audioResource->createInfo.is3D == true ) {
                FMOD_RESULT result = fmodSystem->createSound( audioResource->name.GetCStr(), FMOD_3D | FMOD_NONBLOCKING, 0, &audioResource->sound3D );
                audioResource->sound3D->set3DMinMaxDistance( audioResource->createInfo.minDist, audioResource->createInfo.maxDist );
                ERRCHECK( result );
            }
        }
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
                fmodSpeakers.Remove( speaker->handle );
            }
        }

        return FMOD_OK;
    }

    void WindowsCore::AudioPlay( AudioResource * audio, AudioGroupResource * group ) {
        if ( theGameSettings.noAudio == true ) {
            return;
        }

        FMOD::Channel * channel = nullptr;
        FMOD_RESULT result = fmodSystem->playSound( audio->sound2D, fmodMasterGroup, true, &channel );
        ERRCHECK( result );

        channel->setVolume( audio->volumeMultiplier * 0.3f ); // @HACK:
        channel->setCallback( ChannelCallback );

        AudioSpeakerHandle handle = {};
        AudioSpeaker * speaker = fmodSpeakers.Add( handle );
        if ( speaker != nullptr ) {
            speaker->handle = handle;
            speaker->source = audio;
            speaker->fmodChannel = channel;
            speaker->spawnTime = PlatformGetCurrentTime();
            speaker->sourceGroup = group;
        }

        fmodActives.Add( speaker );

        channel->setUserData( speaker );
        channel->setPaused( false );
    }
  
    void WindowsCore::AudioPlayRandomFromGroup( AudioGroupResource * audioGroup ) {
        const f64 currentTime = GetTheCurrentTime();

        i32 playingCount = 0;
        AudioSpeaker * oldest = nullptr;
        AudioSpeaker * newest = nullptr;
        const i32 activeCount = fmodActives.GetCount();
        for ( i32 speakerIndex = 0; speakerIndex < activeCount; speakerIndex++ ) {
            AudioSpeaker * speaker = fmodActives[ speakerIndex ];
            if ( speaker->sourceGroup == audioGroup ) {
                if ( oldest == nullptr || speaker->spawnTime < oldest->spawnTime ) {
                    oldest = speaker;
                }
                if ( newest == nullptr || speaker->spawnTime > newest->spawnTime ) {
                    newest = speaker;
                }
                playingCount++;
            }
        }

        if ( newest != nullptr ) {
            if ( currentTime - newest->spawnTime < audioGroup->minTimeToPassForAnotherSubmission ) {
                return;
            }
        }

        if ( audioGroup->maxInstances > 0 ) {
            if ( playingCount >= audioGroup->maxInstances ) {
                Assert( oldest != nullptr );

                if ( audioGroup->stealMode == AudioStealMode::NONE ) {
                    return;
                } else if ( audioGroup->stealMode == AudioStealMode::OLDEST ) {
                    oldest->Stop();
                }
            }
        }

        i32 index = Random::Int( audioGroup->sounds.GetCount() );
        AudioPlay( audioGroup->sounds[ index ], audioGroup );
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