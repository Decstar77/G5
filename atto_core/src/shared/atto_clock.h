#pragma once

#include "atto_defines.h"
#include "atto_containers.h"

namespace atto {

    class Core;
    class ScopedClock {
    public:
        ScopedClock( const char * name, Core * core );
        ScopedClock( const LargeString & name, Core * core );
        ~ScopedClock();

    private:
        f64 startTime;
        Core * core;
        LargeString name;
    };
}
