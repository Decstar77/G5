#include "atto_core_windows.h"

namespace atto
{
    void WindowsCore::XAStart() {
        XAudio2Create(&xaEngine, 0);
        xaEngine->CreateMasteringVoice(&xaMasterVoice);
    }

    AudioResource* WindowsCore::ResourceGetAndLoadAudio(const char* name) {
        const i32 audioCount = resources.audios.GetCount();
        for (i32 i = 0; i < audioCount; ++i) {
            if (resources.audios[i].name == name) {
                return &resources.audios[i];
            }
        }

        AudioResource audio = {};
        audio.name = name;
        audio.handle = nullptr;
        return resources.audios.Add(audio);
    }


}