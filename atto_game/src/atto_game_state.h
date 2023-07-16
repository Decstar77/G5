#pragma once
#include "../../atto_core/src/shared/atto_core.h"

namespace atto {

    struct SimState {
        glm::vec2 p1;
        glm::vec2 p2;
    };

    class GameState {
    public:
        void Start(Core* core);
        void UpdateAndRender(Core* core);
        void Shutdown(Core* core);

        void SimStart(Core* core);
        void SimStep(Core* core, const FixedList<i32, 2> &inputs);
        void SimSave(Core* core, void** buffer, i64& size, i64& checkSum);
        void SimLoad(Core* core, void* buffer, i64 size);

    private:
        SmallString     debugText;
        FontResource*   arialFont;
    };
}
