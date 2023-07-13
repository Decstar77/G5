#include "atto_core_windows.h"

#include <al/alc.h>
#include <al/al.h>

namespace atto
{
    bool WindowsCore::ALInitialize() {
        alDevice = alcOpenDevice(nullptr);
        if (alDevice == nullptr) {
            LogOutput(LogLevel::ERR, "Could not open OpenAL device");
            return false;
        }

        alContext = alcCreateContext(alDevice, nullptr);
        if (alContext == nullptr) {
            alcCloseDevice(alDevice);
            LogOutput(LogLevel::ERR, "Could not create OpenAL context");
            return false;
        }

        if (!alcMakeContextCurrent(alContext)) {
            LogOutput(LogLevel::ERR, "Could not make OpenAL context current");
            return false;
        }

        return true;
    }

    void WindowsCore::ALShudown() {
        alcMakeContextCurrent(nullptr);
        alcDestroyContext(alContext);
        alcCloseDevice(alDevice);
    }

    void WindowsCore::ALCheckErrors() {
        ALCenum error = alGetError();
        if (error != AL_NO_ERROR) {
            switch (error)
            {
                case AL_INVALID_NAME:
                    LogOutput(LogLevel::ERR, "AL_INVALID_NAME: a bad name (ID) was passed to an OpenAL ");
                    break;
                case AL_INVALID_ENUM:
                    LogOutput(LogLevel::ERR, "AL_INVALID_ENUM: an invalid enum value was passed to an OpenAL ");
                    break;
                case AL_INVALID_VALUE:
                    LogOutput(LogLevel::ERR, "AL_INVALID_VALUE: an invalid value was passed to an OpenAL ");
                    break;
                case AL_INVALID_OPERATION:
                    LogOutput(LogLevel::ERR, "AL_INVALID_OPERATION: the requested operation is n");
                    break;
                case AL_OUT_OF_MEMORY:
                    LogOutput(LogLevel::ERR, "AL_OUT_OF_MEMORY: the requested operation resulted in OpenAL running out ");
                    break;
                default:
                    LogOutput(LogLevel::ERR, "UNKNOWN AL ERROR: ");
            }
            LogOutput(LogLevel::ERR, "");
        }
    }

    AudioResource* WindowsCore::ResourceGetAndLoadAudio(const char* name) {
        const i32 audioCount = resources.audios.GetCount();
        for (i32 i = 0; i < audioCount; ++i) {
            if (resources.audios[i].name == name) {
                return &resources.audios[i];
            }
        }
        return nullptr;
    }

    AudioSpeaker WindowsCore::AudioPlay(AudioResource* audioResource, f32 volume) {
        return {};
    }
}