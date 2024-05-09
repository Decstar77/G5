#pragma once

#include "atto_defines.h"
#include "atto_containers.h"

namespace atto {

    #define PRINT_LAST_TIME_HERE() { \
    static f32 timer = (f32)PlatformGetCurrentTime();\
    ATTOINFO( "DOne %f", ((f32)PlatformGetCurrentTime() - timer) * 1000.0f );\
    timer = (f32)PlatformGetCurrentTime(); }

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
