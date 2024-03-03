#include "atto_core_windows.h"

#include "audio/AudioFile.h"

#define STB_VORBIS_HEADER_ONLY
#include "stb_vorbis/stb_vorbis.c"

#include <al/alc.h>
#include <al/al.h>

namespace atto {
    bool WindowsCore::ALInitialize() {
        alDevice = alcOpenDevice( nullptr );
        if( alDevice == nullptr ) {
            LogOutput( LogLevel::ERR, "Could not open OpenAL device" );
            return false;
        }

        alContext = alcCreateContext( alDevice, nullptr );
        if( alContext == nullptr ) {
            alcCloseDevice( alDevice );
            LogOutput( LogLevel::ERR, "Could not create OpenAL context" );
            return false;
        }

        if( !alcMakeContextCurrent( alContext ) ) {
            LogOutput( LogLevel::ERR, "Could not make OpenAL context current" );
            return false;
        }

        return true;
    }

    void WindowsCore::ALShudown() {
        alcMakeContextCurrent( nullptr );
        alcDestroyContext( alContext );
        alcCloseDevice( alDevice );
    }

    void WindowsCore::ALCheckErrors() {
        ALCenum error = alGetError();
        if( error != AL_NO_ERROR ) {
            switch( error ) {
                case AL_INVALID_NAME:
                    LogOutput( LogLevel::ERR, "AL_INVALID_NAME: a bad name (ID) was passed to an OpenAL " );
                    break;
                case AL_INVALID_ENUM:
                    LogOutput( LogLevel::ERR, "AL_INVALID_ENUM: an invalid enum value was passed to an OpenAL " );
                    break;
                case AL_INVALID_VALUE:
                    LogOutput( LogLevel::ERR, "AL_INVALID_VALUE: an invalid value was passed to an OpenAL " );
                    break;
                case AL_INVALID_OPERATION:
                    LogOutput( LogLevel::ERR, "AL_INVALID_OPERATION: the requested operation is n" );
                    break;
                case AL_OUT_OF_MEMORY:
                    LogOutput( LogLevel::ERR, "AL_OUT_OF_MEMORY: the requested operation resulted in OpenAL running out " );
                    break;
                default:
                    LogOutput( LogLevel::ERR, "UNKNOWN AL ERROR: " );
            }
            LogOutput( LogLevel::ERR, "" );
        }
    }

    u32 WindowsCore::ALGetFormat( u32 numChannels, u32 bitDepth ) {
        b8 sterio = numChannels > 1;
        if( sterio ) {
            if( bitDepth == 8 ) {
                return AL_FORMAT_STEREO8;
            }
            else if( bitDepth == 16 ) {
                return AL_FORMAT_STEREO16;
            }
        }
        else {
            if( bitDepth == 8 ) {
                return AL_FORMAT_MONO8;
            }
            else if( bitDepth == 16 ) {
                return AL_FORMAT_MONO16;
            }
        }

        LogOutput( LogLevel::ERR, "Unsupported audio format" );

        return 0;
    }

    u32 WindowsCore::ALCreateAudioBuffer( i32 sizeBytes, byte * data, i32 channels, i32 bitDepth, i32 sampleRate ) {
        u32 alFormat = ALGetFormat( channels, bitDepth );

        u32 buffer = 0;
        alGenBuffers( 1, &buffer );
        alBufferData( buffer, alFormat, data, (ALsizei)sizeBytes, (ALsizei)sampleRate );

        ALCheckErrors();

        return buffer;
    }

    bool WindowsCore::ALLoadOGG( const char * file, Win32AudioResource & audioBuffer ) {
        audioBuffer.channels = 0;
        audioBuffer.sampleRate = 0;
        audioBuffer.bitDepth = 16;
        i16 * loadedData = nullptr;
        i32 decoded = stb_vorbis_decode_filename( file, &audioBuffer.channels, &audioBuffer.sampleRate, &loadedData );
        if( loadedData == nullptr ) {
            LogOutput( LogLevel::ERR, "Failed to load audio file %s", file );
            return false;
        }

        audioBuffer.sizeBytes = decoded * audioBuffer.channels * sizeof( i16 );

        audioBuffer.handle = ALCreateAudioBuffer( audioBuffer.sizeBytes, (byte *)loadedData, audioBuffer.channels, audioBuffer.bitDepth, audioBuffer.sampleRate );

        return true;
    }

    bool WindowsCore::ALLoadWAV( const char * file, Win32AudioResource & audioBuffer ) {
        AudioFile<f32> audioFile;
        bool loaded = audioFile.load( file );
        if( !loaded ) {
            LogOutput( LogLevel::ERR, "Failed to load audio file %s", file );
            return false;
        }

        audioBuffer.bitDepth = audioFile.getBitDepth();
        audioBuffer.channels = audioFile.getNumChannels();
        audioBuffer.sampleRate = audioFile.getSampleRate();

        std::vector<u8> data;
        for( i32 sampleIndex = 0; sampleIndex < audioFile.getNumSamplesPerChannel(); sampleIndex++ ) {
            for( i32 channelIndex = 0; channelIndex < audioFile.getNumChannels(); channelIndex++ ) {
                if( audioBuffer.bitDepth == 8 ) {
                    f32 sample = audioFile.samples[ channelIndex ][ sampleIndex ];
                    sample = ( glm::clamp( sample, -1.0f, 1.0f ) + 1.0f ) / 2.0f;
                    u8 s = static_cast<uint8_t> ( sample * 255.0f );
                    data.push_back( s );
                }
                else if( audioBuffer.bitDepth == 16 ) {
                    f32 sample = audioFile.samples[ channelIndex ][ sampleIndex ];
                    sample = glm::clamp( sample, -1.0f, 1.0f );

                    i16 s = static_cast<int16_t> ( sample * 32767. );

                    u8 bytes[ 2 ] = {};
                    bytes[ 1 ] = ( s >> 8 ) & 0xFF;
                    bytes[ 0 ] = s & 0xFF;

                    data.push_back( bytes[ 0 ] );
                    data.push_back( bytes[ 1 ] );
                }
                else if( audioBuffer.bitDepth == 24 ) {
                    f32 sample = audioFile.samples[ channelIndex ][ sampleIndex ];
                    i16 s = NormalizedF64ToI16( (f64)sample );

                    u8 bytes[ 2 ] = {};
                    bytes[ 1 ] = ( s >> 8 ) & 0xFF;
                    bytes[ 0 ] = s & 0xFF;

                    data.push_back( bytes[ 0 ] );
                    data.push_back( bytes[ 1 ] );
                }
                else if( audioBuffer.bitDepth == 32 ) {
                    INVALID_CODE_PATH;
                }
            }
        }

        if( audioBuffer.bitDepth == 24 ) {
            audioBuffer.bitDepth = 16;
        }

        audioBuffer.sizeBytes = (i32)data.size();

        audioBuffer.handle = ALCreateAudioBuffer( audioBuffer.sizeBytes, data.data(), audioBuffer.channels, audioBuffer.bitDepth, audioBuffer.sampleRate );

        return true;
    }

    AudioResource * WindowsCore::ResourceGetAndLoadAudio( const char * name ) {
        const i32 audioCount = resources.audios.GetCount();
        for( i32 i = 0; i < audioCount; ++i ) {
            if( resources.audios[ i ].name == name ) {
                return &resources.audios[ i ];
            }
        }

        Win32AudioResource audio = {};
        audio.name = name;

        LargeString filePath = StringFormat::Large( "%s", name );
        if( audio.name.EndsWith( ".wav" ) ) {
            if( ALLoadWAV( filePath.GetCStr(), audio ) == false ) {
                LogOutput( LogLevel::ERR, "AUDIO :: Could not load %s", name );
            }
        }
        else if( audio.name.EndsWith( ".ogg" ) ) {
            if( ALLoadOGG( filePath.GetCStr(), audio ) == false ) {
                LogOutput( LogLevel::ERR, "AUDIO :: Could not load %s", name );
            }
        }
        else {
            LogOutput( LogLevel::ERR, "AUDIO :: Unsupported format" );
        }

        return resources.audios.Add( audio );
    }

    AudioSpeaker WindowsCore::AudioPlay( AudioResource * audio, f32 volume, bool looping ) {
        if( theGameSettings.noAudio ) {
            return {};
        }

        Win32AudioResource * a = (Win32AudioResource *)audio;

        volume = volume * theGameSettings.masterVolume;

        const i32 speakerCount = alSpeakers.GetCount();
        for( i32 speakerIndex = 0; speakerIndex < speakerCount; ++speakerIndex ) {
            AudioSpeaker & speaker = alSpeakers[ speakerIndex ];
            if( speaker.sourceHandle == 0 ) {
                continue;
            }

            ALint state = {};
            alGetSourcei( speaker.sourceHandle, AL_SOURCE_STATE, &state );
            if( state == AL_STOPPED ) {
                alSourcei( speaker.sourceHandle, AL_BUFFER, a->handle );
                alSourcei( speaker.sourceHandle, AL_LOOPING, looping ? AL_TRUE : AL_FALSE );
                alSourcef( speaker.sourceHandle, AL_GAIN, volume );
                alSource3f( speaker.sourceHandle, AL_POSITION, 0.0f, 0.0f, 0.0f );
                alSource3f( speaker.sourceHandle, AL_VELOCITY, 0.0f, 0.0f, 0.0f );
                alSourcePlay( speaker.sourceHandle );
                ALCheckErrors();

                return speaker;
            }
        }

        AudioSpeaker speaker = {};
        if( alSpeakers.GetCount() < alSpeakers.GetCapcity() ) {
            speaker.index = alSpeakers.GetCount();
            alGenSources( 1, &speaker.sourceHandle );
            alSourcei( speaker.sourceHandle, AL_BUFFER, a->handle );
            alSourcei( speaker.sourceHandle, AL_LOOPING, looping ? AL_TRUE : AL_FALSE );
            alSourcef( speaker.sourceHandle, AL_GAIN, volume );
            alSource3f( speaker.sourceHandle, AL_POSITION, 0.0f, 0.0f, 0.0f );
            alSource3f( speaker.sourceHandle, AL_VELOCITY, 0.0f, 0.0f, 0.0f );
            alSourcePlay( speaker.sourceHandle );
            ALCheckErrors();
            alSpeakers.Add( speaker );
        }
        else {
            LogOutput( LogLevel::WARN, "All the speakers are busy!!" );
        }

        return speaker;
    }

    AudioSpeaker WindowsCore::AudioPlay( AudioResource * audioResource, glm::vec2 pos, glm::vec2 vel, f32 volume /*= 1.0f*/, bool looping /*= false */ ) {
        if( theGameSettings.noAudio ) {
            return {};
        }

        AudioSpeaker speaker = AudioPlay( audioResource, volume, looping );

        alSource3f( speaker.sourceHandle, AL_POSITION, pos.x, pos.y, 0.0f );
        alSource3f( speaker.sourceHandle, AL_VELOCITY, vel.x, vel.y, 0.0f );
        ALCheckErrors();

        return speaker;
    }

    void WindowsCore::AudioSetListener( glm::vec2 pos, glm::vec2 vel ) {
        if( theGameSettings.noAudio ) {
            return;
        }

        listenerPos = pos;
        listenerVel = vel;

        alListener3f( AL_POSITION, listenerPos.x, listenerPos.y, 0.0f );
        alListener3f( AL_VELOCITY, listenerVel.x, listenerVel.y, 0.0f );
        ALCheckErrors();
    }
}