#pragma once

#include "atto_defines.h"
#include "atto_random.h"
#include "atto_reflection.h"

namespace atto {

    inline i16 NormalizedF64ToI16( f64 v ) {
        return static_cast<i16>( v * 32767.0 );
    }

    inline u16 SafeTruncateU32( u32 v ) {
        Assert( v <= 0xFFFF );
        return static_cast<u16>( v );
    }

    inline i32 PosIndexToFlatIndex( i32 x, i32 y, i32 width ) {
        return y * width + x;
    }

    inline i32 FlatIndexToPosIndex( i32 flat, i32 width, i32 & x, i32 & y ) {
        x = flat % width;
        y = flat / width;
        return flat;
    }

    inline i32 PosIndexToFlatIndex( i32 x, i32 y, i32 z, i32 width, i32 height ) {
        return z * width * height + y * width + x;
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

    struct BoxBounds2D {
        glm::vec2 min;
        glm::vec2 max;

        f32                     GetWidth() const;
        f32                     GetHeight() const;
        glm::vec2               GetCenter() const;
        glm::vec2               GetSize() const;

        void                    Translate( const glm::vec2 & translation );

        bool                    Intersects( const BoxBounds2D & other ) const;
        bool                    Intersects( const Circle & other ) const;

        bool                    Collision( const BoxBounds2D & other, Manifold2D & manifold ) const;
        bool                    Collision( const Circle & other, Manifold2D & manifold ) const;

        bool                    Contains( const glm::vec2 & point ) const;
        void                    Expand( f32 mul );

        void                    CreateFromCenterSize( const glm::vec2 & center, const glm::vec2 & size );

        REFLECT();
    };
    
    struct BoxBounds {
        glm::vec3 min;
        glm::vec3 max;

        f32                     GetWidth() const;
        f32                     GetHeight() const;
        f32                     GetDepth() const;
        glm::vec3               GetCenter() const;
        glm::vec3               GetSize() const;
        bool                    Contains( glm::vec3 point ) const;

        void                    Translate( glm::vec3 translation );
        void                    Expand( glm::vec3 p ); // expand to contain point
        void                    CreateFromCenterSize( glm::vec3 center, glm::vec3  size );
    };

    enum ColliderType {
        COLLIDER_TYPE_NONE,
        COLLIDER_TYPE_CIRCLE,
        COLLIDER_TYPE_SPHERE,
        COLLIDER_TYPE_BOX,
        COLLIDER_TYPE_PLANE,
        COLLIDER_TYPE_TRIANGLE,
    };

    struct Collider2D {
        ColliderType type;
        union {
            Circle circle;
            BoxBounds2D box;
        };

        void                    Translate( const glm::vec2 & translation );
        bool                    Contains( const glm::vec2 & point ) const;
        bool                    Intersects( const BoxBounds2D & bounds ) const;
        bool                    Intersects( const Collider2D & other ) const;
        bool                    Collision( const Collider2D & other, Manifold2D & manifold ) const;
        bool                    Collision( const BoxBounds2D & box, Manifold2D & manifold ) const;
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
                glm::vec3 min;
                glm::vec3 max;
            } box;
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

        void Translate( glm::vec3 p );
    };

    glm::vec3 ClosestPoint_Sphere( glm::vec3 c, f32 r, glm::vec3 p );
    glm::vec3 ClosestPoint_Box( glm::vec3 min, glm::vec3 max, glm::vec3 p );
    glm::vec3 ClosestPoint_Plane( glm::vec3 c, glm::vec3 n, glm::vec3 p );
    glm::vec3 ClosestPoint_LineSegment( glm::vec3 a, glm::vec3 b, glm::vec3 p );
    glm::vec3 ClosestPoint_Triangle( glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, glm::vec3 n, glm::vec3 p );
    glm::vec3 ClosestPoint_Triangle( Collider tri, glm::vec3 p );
    
    bool CollisionCheck_SphereVsSphere( glm::vec3 c1, f32 r1, glm::vec3 c2, f32 r2, Manifold & manifold );
    bool CollisionCheck_SphereVsBox( glm::vec3 c, f32 r, glm::vec3 min, glm::vec3 max, Manifold & manifold );
    bool CollisionCheck_SphereVsTri( glm::vec3 c, f32 r, glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, glm::vec3 n, Manifold & manifold );
    bool CollisionCheck_SphereVsPlane( Collider sphere, Collider plane, Manifold & manifold );
    bool CollisionCheck_SphereVsTri( Collider sphere, Collider tri, Manifold & manifold );

}