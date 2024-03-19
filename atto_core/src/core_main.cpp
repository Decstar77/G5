
#include "win32/atto_core_windows.h"

using namespace atto;



/*
MEMES:
    An ability that scrambles your opponesnts selections.... :D :D :D

*/

/*
* @TODO: Fix circle rendering
* @TODO: Fix alpha padding issue
* @TODO: Live resource reloading.
* @TODO: Store all resources in a single file.
* @TODO: 3D Audio support.
* @TODO: Fix bullet despawning
*/

/*
IDEAS:
    You get cloned ?
    CAMERA SHAKE
    HEALTH BAR
    SHEILD BAR
    ENERGY BAR

    EACH BIOOM will drop different parts. 

    BIOOOOMS:
        - Homebase:
            - Augment station

        - Ship:
            Tile sheet:
                - Ventilation
                - Control panel
            Props:
                - Charging station
                - Cables
                - Pipes
                - Storage
                - Monitor/Datapad
                - Escape pod

            Enemies:
                Shield drones: ( Shield effect + particles )
                Kamikazy drones: ( Explosition + Particles )
                Wall turret: 
                Ground turret: ( Explosition + Particles )
                Droikedia:

            Boss:
                - Captains bolts

        - Desert:
            Enemies:
                Sand worm
        
        - Forest:
            Enemies:
                -

        - Snow: 
            Enemies:
                -

        - Volcano:
            Enemies:
                    -


FUNNY:
ACHIVEMTN: You've been granted a random perk: Dead.
ACHIVEMTN: Hey, real quick, whats the radius on that?: "Yes."
*/


int main( int argc, char ** argv ) {
    WindowsCore * core = new WindowsCore();
    core->Run( argc, argv );
    return 0;
}


