
#include "../shared/atto_core.h"

namespace atto {
    class GameLogicAndState : public IGameLogicAndState {
    public:
        void Start(Core* core) override;
        void UpdateAndRender(Core* core, ISimLogicAndState* sim) override;
        void Shutdown(Core* core) override;

        SmallString     debugText;
        FontResource* arialFont;

        glm::vec2 p1VisPos;
        glm::vec2 p2VisPos;
    };
}

