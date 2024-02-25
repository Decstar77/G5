#pragma once

#include "atto_defines.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/vector_angle.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/norm.hpp>

namespace atto {
    class Random {
    public:
        static f32 Float();
        static f32 Float( f32 min, f32 max );
        static i32 Int( i32 min, i32 max );
        static i32 Int( i32 max );

        static glm::vec2 Vec2( glm::vec2 min, glm::vec2 max );
    };
}

