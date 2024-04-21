
#if defined( __linux__ )

#include "../../shared/atto_defines.h"

namespace atto {
    void PlatformAssertionFailed( const char * msg, const char * file, const char * func, int line ) {

    }
    
    void PlatformLogMessage( const char * message, u8 colour ) {

    }

    void PlatformErrorBox( const char * msg ) {

    }

    u64 PlatformGetFileLastWriteTime( const char * fileName ) {
        return 0;
    }
}

#endif
