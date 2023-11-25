
#include "win32/atto_core_windows.h"

using namespace atto;

/*
* TODO: Need to ui rendering (weekend task)
* TODO: Better crash support, such as dump files + call stack. (night task)
* TODO: Input replays (night task)
* TODO: Better logging, log files for ggpo ect (night task)
*/


int main(int argc, char** argv) {
    WindowsCore* core = new WindowsCore();
    core->Run(argc, argv);
    return 0;
}
