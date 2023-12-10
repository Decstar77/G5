#pragma once

#include "atto_defines.h"
namespace atto {
    class Random {
    public:
        static f32 Float();
        static f32 Float( f32 min, f32 max );
        static i32 Int( i32 min, i32 max );
        static i32 Int( i32 max );
    };
}

