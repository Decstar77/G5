
#include "win32/atto_core_windows.h"

using namespace atto;

/*
* TODO: Add a way to draw 3d lines. ( DONE )!
* TODO: Make sure the normals are correct for the block maps ( DONE )!
* TODO: Add mip-maps and anisotropic filtering ( DONE )!
* TODO: Add basic collisions with the map tiles ( DONE )!
* TODO: Make saving a map layout work. ( DONE )!
* TODO: Refactor the map + entity.
* TODO: Make the editor override the local player
* TODO: Load and draw a 3D model
* TODO: 
*/


int main(int argc, char** argv) {
    WindowsCore* core = new WindowsCore();
    core->Run(argc, argv);
    return 0;
}


