
#include "win32/atto_core_windows.h"

using namespace atto;


/*
* @TODO: Fix circle rendering
* @TODO: Fix alpha padding issue
* @TODO: Live resource reloading.
* @TODO: Store all resources in a single file.
* @TODO: 3D Audio support.
*/

/*
IDEAS:

    BIOOOOMS:
        - Ship:
            Enemies:
                Healer/Shield drones
                Kamikazy drones
                Wall turret
                Ground turret
                Rover turret
                Droikedia
                Battle droid

        

FUNNY:
ACHIVEMTN: You've been granted a random perk: Dead.
ACHIVEMTN: Hey, real quick, whats the radius on that?: "Yes."
*/


int main( int argc, char ** argv ) {
    WindowsCore * core = new WindowsCore();
    core->Run( argc, argv );
    return 0;
}


