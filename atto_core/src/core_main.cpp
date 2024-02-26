
#include "win32/atto_core_windows.h"

using namespace atto;


int main( int argc, char ** argv ) {
    WindowsCore * core = new WindowsCore();
    core->Run( argc, argv );
    return 0;
}


