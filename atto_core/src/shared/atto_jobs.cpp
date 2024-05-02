#include "atto_jobs.h"

namespace atto {
    static enki::TaskScheduler taskScheduler = {};

    void JobInitialize() {
        taskScheduler.Initialize( 4 );
    }

    void JobShutdown() {
        taskScheduler.ShutdownNow();
    }

    void JobAdd( enki::ITaskSet * task ) {
        taskScheduler.AddTaskSetToPipe( task );
    }

    void JobWaitFor( enki::ITaskSet * task ) {
        taskScheduler.WaitforTaskSet( task );
    }
}

