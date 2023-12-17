#pragma once

#include "atto_defines.h"
#include "atto_random.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/vector_angle.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/norm.hpp>

namespace atto {

    inline i16 NormalizedF64ToI16( f64 v ) {
        return static_cast<i16>( v * 32767.0 );
    }

    struct Manifold2D {
        f32 penetration;
        glm::vec2 pointA;
        glm::vec2 pointB;
        glm::vec2 normal;

        inline void Flip() {
            normal = -normal;
            glm::vec2 temp = pointA;
            pointA = pointB;
            pointB = temp;
        }
    };


    struct Circle {
        glm::vec2   pos;
        f32         rad;

        bool                    Intersects( const Circle & circle ) const;
        bool                    Collision( const Circle & other, Manifold2D & manifold ) const;
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

        bool                    Collision( const BoxBounds & other, Manifold2D & manifold ) const;
        bool                    Collision( const Circle & other, Manifold2D & manifold ) const;

        bool                    Contains( const glm::vec2 & point ) const;
        void                    Expand( f32 mul );

        void                    CreateFromCenterSize( const glm::vec2 & center, const glm::vec2 & size );
    };

    enum ColliderType {
        COLLIDER_TYPE_CIRCLE,
        COLLIDER_TYPE_BOX,
        COLLIDER_TYPE_SHPERE,
        COLLIDER_TYPE_PLANE,
        COLLIDER_TYPE_TRIANGLE,
    };

    struct Collider2D {
        ColliderType type;
        union {
            Circle circle;
            BoxBounds box;
        };

        void                    Translate( const glm::vec2 & translation );
        bool                    Contains( const glm::vec2 & point ) const;
        bool                    Intersects( const Collider2D & other ) const;
        bool                    Collision( const Collider2D & other, Manifold2D & manifold ) const;
    };

    struct Manifold {
        f32 penetration;
        glm::vec3 pointA;
        glm::vec3 pointB;
        glm::vec3 normal;
    };

    struct Collider {
        ColliderType type;
        union {
            struct {
                glm::vec3 c;
                f32 r;
            } sphere;
            struct {
                glm::vec3 c;
                glm::vec3 n;
            } plane;
            struct {
                glm::vec3 p1;
                glm::vec3 p2;
                glm::vec3 p3;
                glm::vec3 n;
            } tri;
        };
    };

    glm::vec3 ClosestPoint_Sphere( glm::vec3 c, f32 r, glm::vec3 p );
    glm::vec3 ClosestPoint_Plane( glm::vec3 c, glm::vec3 n, glm::vec3 p );
    glm::vec3 ClosestPoint_LineSegment( glm::vec3 a, glm::vec3 b, glm::vec3 p );
    glm::vec3 ClosestPoint_Triangle( Collider tri, glm::vec3 p );
    
    bool CollisionCheck_SphereVsPlane( Collider sphere, Collider plane, Manifold & manifold );
    bool CollisionCheck_SphereVsTri( Collider sphere, Collider tri, Manifold & manifold );

}