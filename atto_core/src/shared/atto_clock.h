#pragma once

#include "atto_defines.h"
#include "atto_containers.h"

namespace atto {

    class ScopedClock {
    public:
        ScopedClock( const char * name );
        ScopedClock( const LargeString & name );
        ~ScopedClock();

    private:
        f64 startTime;
        LargeString name;
    };
}
