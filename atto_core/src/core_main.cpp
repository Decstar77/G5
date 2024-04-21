
#include "platform/win32/atto_win32_core.h"

using namespace atto;

/*
MEMES:
    An ability that scrambles your opponesnts selections.... :D :D :D
    Combine solar systems
    Smash solar systems together 
    ACHIVEMTN: You've been granted a random perk: Dead.
    ACHIVEMTN: Hey, real quick, whats the radius on that?: "Yes."
*/

/*
* @TODO: Move to fixed point
* @TODO: Y-sorting
* @TODO: Fix circle rendering
* @TODO: Live resource reloading.
* @TODO: Store all resources in a single file.
* @TODO: 3D Audio support.
* @TODO: Move to vulkan rendering
*/

/*
IDEAS:
    You get cloned ?
    CAMERA SHAKE
    HEALTH BAR
    SHEILD BAR
    ENERGY BAR

*/


int main( int argc, char ** argv ) {
    WindowsCore * core = new WindowsCore();
    core->Run( argc, argv );
    return 0;
}


