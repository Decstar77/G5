/* -----------------------------------------------------------------------
 * GGPO.net (http://ggpo.net)  -  Copyright 2009 GroundStorm Studios, LLC.
 *
 * Use of this software is governed by the MIT license that can be found
 * in the LICENSE file.
 */

#include "platform_windows.h"
#include <windows.h>

Platform::ProcessID Platform::GetProcessID() {
    return GetCurrentProcessId();
}

void Platform::AssertFailed( char * msg ) {
    MessageBoxA( NULL, msg, "GGPO Assertion Failed", MB_OK | MB_ICONEXCLAMATION );
    __debugbreak();
}

uint32 Platform::GetCurrentTimeMS() {
    return timeGetTime();
}

int Platform::GetConfigInt( const char * name ) {
    char buf[ 1024 ];
    if( GetEnvironmentVariableA( name, buf, ARRAY_SIZE( buf ) ) == 0 ) {
        return 0;
    }
    return atoi( buf );
}

bool Platform::GetConfigBool( const char * name ) {
    char buf[ 1024 ];
    if( GetEnvironmentVariableA( name, buf, ARRAY_SIZE( buf ) ) == 0 ) {
        return false;
    }
    return atoi( buf ) != 0 || _stricmp( buf, "true" ) == 0;
}
