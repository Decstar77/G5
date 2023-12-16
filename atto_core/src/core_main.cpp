
#include "win32/atto_core_windows.h"


namespace atto {

    inline FixedList<int, 120> vs = {};
    
    class SomethingClass {
    public:
        SomethingClass() {
            vs.Add( 1 );
        }
    };

    inline SomethingClass sc1;
    inline SomethingClass sc2;
    inline SomethingClass sc3;


    struct Something {
        int a;
        int b;
        int c;
    };

    REFL_DECLARE( Something );
    REFL_VAR( Something, a );
    REFL_VAR( Something, b );
    REFL_VAR( Something, c );
    

    struct SomethingElse {
        Something s;
        int b;
        SmallString c;
        FixedList<i32, 20> e;
        FixedList<Something, 5> f;
    };

    REFL_DECLARE( SomethingElse );
    RELF_VAR( SomethingElse, Something, s );
    REFL_VAR( SomethingElse, b );
    REFL_VAR( SomethingElse, c );
    REFL_VAR( SomethingElse, e );
    REFL_VAR( SomethingElse, FixedList<Something, 5>, f );
}

using namespace atto;

/*
* TODO: Need to ui rendering (weekend task)
* TODO: Better crash support, such as dump files + call stack. (night task)
* TODO: Input replays (night task)
*/


int main(int argc, char** argv) {
    SomethingElse s = {};
    s.s.b = 3;
    s.c = "hello";
    s.e.Add( 3 );
    s.e.Add( 6 );
    REFL_WRITE_JSON( "tester.json", SomethingElse, s );

    SomethingElse e = { };
    REFL_READ_JSON( "tester.json", SomethingElse, e );

    

    WindowsCore* core = new WindowsCore();
    core->Run(argc, argv);
    return 0;
}
