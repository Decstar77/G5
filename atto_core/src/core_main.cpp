
#include "win32/atto_core_windows.h"

using namespace atto;

/*
* TODO: Add a way to draw 3d lines.
* TODO: Make sure the normals are correct for the block maps !
* TODO: Add mip-maps and anisotropic filtering !
* TODO: Add collisions with the map tiles
* TODO: Make saving a map layout work.
*/


int main(int argc, char** argv) {
    WindowsCore* core = new WindowsCore();
    core->Run(argc, argv);
    return 0;
}


