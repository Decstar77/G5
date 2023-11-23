
#include "win32/atto_core_windows.h"

using namespace atto;

/*
* TODO: There is probably a way to refactor out the peer handle of ggpo and use the player number instead (weekend task)
* TODO: Need to ui rendering (weekend task)
* TODO: Better crash support, such as dump files + call stack. (night task)
* TODO: Input replays (night task)
* TODO: Better logging, log files for ggpo ect (night task)
*/

struct Something {
    i32 a;
    SmallString b;
    glm::vec2 c;
    FixedList<f32, 5> d;
};

REFL_DECLARE( Something );
REFL_VAR( Something, a );
REFL_VAR( Something, b );
REFL_VAR( Something, c );
REFL_VAR( Something, d );


int main(int argc, char** argv) {

    Something s = {};
    s.a = 2;
    s.b = SmallString::FromLiteral( "Hello oskads" );
    s.c = glm::vec2( 0.23f, 9.12f );
    REFL_WRITE_JSON( "something.json", Something, s );
    Something e = {};
    REFL_READ_JSON( "something.json", Something, e );


    WindowsCore* core = new WindowsCore();
    core->Run(argc, argv);
    return 0;
}
