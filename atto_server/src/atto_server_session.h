#pragma once

#include "../../atto_core/src/shared/atto_defines.h"
#include "../../atto_core/src/shared/atto_containers.h"
#include "../../atto_core/src/shared/atto_binary_file.h"
#include "../../atto_core/src/shared/atto_network.h"
#include "../../atto_core/src/shared/atto_clock.h"
#include "../../atto_core/src/shared/atto_math.h"
#include "../../atto_core/src/shared/enki_task_scheduler.h"
#include "../../atto_core/src/shared/atto_reflection.h"
#include "../../atto_core/src/shared/atto_logging.h"
#include "../../atto_core/src/shared/atto_rpc.h"

#include "../../atto_core/src/sim/atto_sim_map.h"

namespace atto {

    class Session {
    public:
        u64 peer1;
        u64 peer2;

        MapActionBuffer inAction;
        MapActionBuffer outAction;

        SimMap simMap;

        f64 timeAccumulator = 0.0;

        void Initialize();
        void Update( f32 dt );
    };
}

