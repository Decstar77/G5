
#include "win32/atto_core_windows.h"

using namespace atto;


/*
* @TODO: Fix circle rendering
* @TODO: Fix alpha padding issue
* @TODO: Live resource reloading.
* @TODO: Store all resources in a single file.
* @TODO: 3D Audio support.
*/

int main( int argc, char ** argv ) {
    WindowsCore * core = new WindowsCore();
    core->Run( argc, argv );
    return 0;
}


