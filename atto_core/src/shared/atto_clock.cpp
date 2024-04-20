#include "atto_clock.h"
#include "atto_core.h"

namespace atto {
    ScopedClock::ScopedClock( const char * text, Core * core ) : core( core ), name( LargeString::FromLiteral( text ) ), startTime( 0 ) {
        startTime = core->GetTheCurrentTime();
    }

    ScopedClock::ScopedClock( const LargeString & name, Core * core ) : core( core ), name( name ), startTime( 0 ) {
        startTime = core->GetTheCurrentTime();;
    }

    ScopedClock::~ScopedClock() {
        f64 endTime = core->GetTheCurrentTime();
        f64 seconds = endTime - startTime;
        f64 dt = seconds * 1000.0;
        core->LogOutput( LogLevel::INFO, "ScopedClock '%s' time ms = %f", name.GetCStr(), (f32)dt );
    }
}