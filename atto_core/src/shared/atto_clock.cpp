#include "atto_clock.h"
#include "atto_core.h"

namespace atto {
    ScopedClock::ScopedClock( const char * text, Core * core ) : core( core ), name( SmallString::FromLiteral( text ) ), startTime( 0 ) {
        startTime = core->GetTheCurrentTime();
    }

    ScopedClock::~ScopedClock() {
        f64 endTime = core->GetTheCurrentTime();
        f64 dt = endTime - startTime;
        core->LogOutput( LogLevel::INFO, "ScopedClock '%s' time = %f", name.GetCStr(), (f32)dt );
    }
}