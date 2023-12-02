#pragma once

#include "atto_defines.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/vector_angle.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/norm.hpp>

namespace atto {

    inline i16 NormalizedF64ToI16( f64 v ) {
        return static_cast<i16>( v * 32767.0 );
    }

    struct Manifold {
        f32 penetration;
        glm::vec2 pointA;
        glm::vec2 pointB;
        glm::vec2 normal;
    };


    struct Circle {
        glm::vec2   pos;
        f32         rad;

        bool                    Intersects( const Circle & circle );
        bool                    Collision( const Circle & other, Manifold & manifold ) const;
        bool                    Contains( glm::vec2 point ) const;
        static bool             Contains( glm::vec2 c, f32 r, glm::vec2 point );
        static Circle           Create( glm::vec2 pos, f32 rad );
    };

    struct BoxBounds {
        glm::vec2 min;
        glm::vec2 max;

        f32                     GetWidth() const;
        f32                     GetHeight() const;
        glm::vec2               GetCenter() const;
        glm::vec2               GetSize() const;

        void                    Translate( const glm::vec2 & translation );

        bool                    Intersects( const BoxBounds & other ) const;
        bool                    Intersects( const Circle & other ) const;

        bool                    Collision( const BoxBounds & other, Manifold & manifold ) const;
        bool                    Contains( const glm::vec2 & point ) const;
        void                    Expand( f32 mul );

        void                    CreateFromCenterSize( const glm::vec2 & center, const glm::vec2 & size );
    };
}