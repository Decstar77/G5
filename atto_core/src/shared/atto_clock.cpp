#include "atto_clock.h"
#include "atto_logging.h"

namespace atto {

    ScopedClock::ScopedClock( const char * text ) : name( LargeString::FromLiteral( text ) ), startTime( 0 ) {
        startTime = PlatformGetCurrentTime();
    }

    ScopedClock::ScopedClock( const LargeString & name ) : name( name ), startTime( 0 ) {
        startTime = PlatformGetCurrentTime();
    }

    ScopedClock::~ScopedClock() {
        f64 endTime = PlatformGetCurrentTime();
        f64 seconds = endTime - startTime;
        f64 dt = seconds * 1000.0;
        ATTOINFO( "ScopedClock '%s' time ms = %f", name.GetCStr(), (f32)dt );
    }
}