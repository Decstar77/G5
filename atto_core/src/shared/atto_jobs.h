#pragma once

#include "enki_task_scheduler.h"

namespace atto {
    void JobInitialize();
    void JobAdd( enki::ITaskSet * task );
    void JobWaitFor( enki::ITaskSet * task );
    void JobShutdown();
}


