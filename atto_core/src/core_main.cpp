
#include "win32/atto_core_windows.h"

using namespace atto;

int main() {
    Core* core = new WindowsCore();
    core->Run();
    return 0;
}
