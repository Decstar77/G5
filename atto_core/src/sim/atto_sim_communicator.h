#pragma once
#include "../shared/atto_defines.h"
#include "../shared/atto_math.h"

#include "atto_sim_actions.h"

namespace atto {
    class SimCommunicator {
    public:
        virtual void SendToAllClients( MapActionBuffer * actionBuffer ) = 0;
    };
}
