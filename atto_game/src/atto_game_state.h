#pragma once
#include "../../atto_core/src/shared/atto_core.h"

namespace atto {
    class GameState {
    public:
        void Start(Core* core);
        void UpdateAndRender(Core* core);
        void Shutdown(Core* core);

    private:
        i32 stuff;
        i32 s;
    };
}
