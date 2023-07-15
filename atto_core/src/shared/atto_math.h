#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/vector_angle.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/norm.hpp>

namespace atto {
    inline i16 NormalizedF64ToI16(f64 v) {
        return static_cast<i16>(v * 32767.0);
    }
}