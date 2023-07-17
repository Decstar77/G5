
#include "../shared/atto_core.h"

namespace atto {
    class SimLogicAndState : public ISimLogicAndState {
    public:
        void Start(Core* core) override;
        void Step(Core* core, i32 i1, i32 i2) override;
    };
}

