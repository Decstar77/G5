#pragma once

#include "atto_defines.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/vector_angle.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/norm.hpp>

#include <fpm/fixed.hpp>
#include <fpm/math.hpp>
#include <fpm/ios.hpp>

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

    inline f32 LerpAngle( f32 a, f32 b, f32 t ) {
            f32 diff = b - a;
        if( diff > glm::pi<f32>() ) {
            b -= glm::two_pi<f32>();
        }
        else if( diff < -glm::pi<f32>() ) {
            b += glm::two_pi<f32>();
        }
        return a + t * ( b - a );
    }

    inline glm::vec2 Truncate( glm::vec2 v, f32 maxMag ) {
        f32 l = glm::length( v );
        if( l > maxMag ) {
            return ( v / l ) * maxMag;
        }
        return v;
    }

    inline glm::vec2 LeftPerp( glm::vec2 v ) {
        return glm::vec2( -v.y, v.x );
    }

    typedef fpm::fixed<std::int32_t, std::int64_t, 6> fp;

    constexpr fp FP_MAX = ( std::numeric_limits<fp>::max() );
    constexpr fp FP_MIN = ( std::numeric_limits<fp>::min() );
    constexpr fp FP_EPI = ( std::numeric_limits<fp>::epsilon() ); // @NOTE: 0.01.
    constexpr fp FP_E = fp::e();
    constexpr fp FP_PI = fp::pi();
    constexpr fp FP_2PI = fp::two_pi();
    constexpr fp FP_HALF_PI = fp::half_pi();

    inline constexpr fp Fp( f32 f ) { return fp( f ); }
    inline constexpr fp Fp( i32 f ) { return fp( f ); }
    inline constexpr fp ToFP( f32 f ) { return fp( f ); }
    inline constexpr fp ToFP( i32 f ) { return fp( f ); }
    inline constexpr i32 ToInt( fp f ) { return static_cast<i32>( f ); }
    inline constexpr f32 ToFloat( fp f ) { return static_cast<f32>( f ); }

    constexpr i32 FP_EPI_MAX = ToInt( FP_MAX );
    constexpr float FP_EPI_FLOAT = ToFloat( FP_EPI );

    struct fp2 {
        fp x;
        fp y;
    };

    inline constexpr fp2 Fp2( f32 x, f32 y ) { return { Fp( x ), Fp( y ) }; }
    inline constexpr fp2 Fp2( i32 x, i32 y ) { return { Fp( x ), Fp( y ) }; }
    inline constexpr fp2 Fp2( glm::vec2 v ) { return { Fp( v.x ), Fp( v.y ) }; }


    glm::vec2   ToVec2( const fp2 & v );
    fp2         ToFP2( const glm::vec2 & v );

    fp          FpAbs( fp f );
    fp          FpRound( fp f );
    fp          FpSin( fp f );
    fp          FpCos( fp f );
    fp          FpTan( fp f );
    fp          FpASin( fp f );
    fp          FpACos( fp f );
    fp          FpATan2( fp y, fp x );
    fp          FpMin( fp a, fp b );
    fp          FpMax( fp a, fp b );
    fp          FpClamp( fp f, fp min, fp max );
    fp          FpLerp( fp a, fp b, fp t );
    fp2         FpAbs( fp2 v );
    fp          FpLength( fp2 v );
    fp          FpLength2( fp2 v );
    fp          FpDistance( fp2 v1, fp2 v2 );
    fp          FpDistance2( fp2 v1, fp2 v2 );
    fp2         FpNormalize( fp2 v );
    fp2         FpTruncateLength( fp2, fp max );
    fp2         FpLeftPerp( fp2 v );
    fp2         FpRotate( fp2 v, fp angle );

    fp2         operator+( const fp2 & a, const fp2 & b );
    fp2         operator-( const fp2 & a, const fp2 & b );
    fp2         operator*( const fp2 & a, const fp & b );
    fp2         operator/( const fp2 & a, const fp & b );
    fp2         operator*( const fp & a, const fp2 & b );
    fp2         operator/( const fp & a, const fp2 & b );
    fp2         operator-( const fp2 & a );
    bool        operator==( const fp2 & a, const fp2 & b );
    bool        operator!=( const fp2 & a, const fp2 & b );

    enum ColliderType {
        COLLIDER_TYPE_NONE,
        COLLIDER_TYPE_CIRCLE,
        COLLIDER_TYPE_SPHERE,
        COLLIDER_TYPE_AXIS_BOX,
        COLLIDER_TYPE_PLANE,
        COLLIDER_TYPE_TRIANGLE,
    };

    struct FpCircle {
        fp  rad;
        fp2 pos;
    };

    struct FpAxisBox {
        fp2 min;
        fp2 max;
    };

    struct FpManifold {
        fp penetration;
        fp2 pointA;
        fp2 pointB;
        fp2 normal;
    };

    FpCircle        FpCircleCreate( fp2 pos, fp rad );
    bool            FpCircleIntersects( FpCircle a, FpCircle b );
    fp              FpColliderSurfaceDistance( FpCircle a, FpCircle b );
    bool            FpCircleCollision( FpCircle a, FpCircle b, FpManifold & manifold );
    bool            FpCircleContains( FpCircle a, fp2 point );

    FpAxisBox       FpAxisBoxCreateFromMinMax( fp2 min, fp2 max );
    FpAxisBox       FpAxisBoxCreateFromCenterSize( fp2 center, fp2 size );
    fp              FpAxisBoxGetWidth( FpAxisBox b );
    fp              FpAxisBoxGetHeight( FpAxisBox b );
    fp2             FpAxisBoxGetCenter( FpAxisBox b );
    fp2             FpAxisBoxGetSize( FpAxisBox b );
    void            FpAxisBoxTranslate( FpAxisBox * b, fp2 translation );
    fp              FpColliderSurfaceDistance( FpAxisBox a, FpAxisBox b );
    fp              FpColliderSurfaceDistance( FpAxisBox a, FpCircle c );
    bool            FpAxisBoxIntersects( FpAxisBox a, FpAxisBox b );
    bool            FpAxisBoxIntersects( FpAxisBox b, FpCircle c );
    fp2             FpClosestPointBox( FpAxisBox b, fp2 p );
    bool            FpAxisBoxCollision( FpAxisBox a, FpAxisBox b, FpManifold & manifold );
    bool            FpAxisBoxContains( FpAxisBox a , fp2 p );

    struct FpCollider {
        ColliderType type;
        union {
            FpCircle circle;
            FpAxisBox box;
        };
    };

    void            FpColliderSetPos( FpCollider * a, fp2 p );
    fp              FpColliderSurfaceDistance( FpCollider a, FpCollider b );
    bool            FpColliderIntersects( FpCollider a, FpCircle b );
    bool            FpColliderIntersects( FpCollider a, FpAxisBox b );
    bool            FpColliderIntersects( FpCollider a, FpCollider b );

    struct RNG {
        i32 index;
        i32 numbers[1000];
    };

    void    RNGCreate( RNG * rng, i32 seed );
    i32     RNGInt( RNG * rng, i32 min, i32 max );
    i32     RNGInt( RNG * rng, i32 max );
    fp      RNGFp( RNG * rng );
    fp      RNGFp( RNG * rng, fp min, fp max );
    fp2     RNGFp2( RNG * rng, fp min, fp max );

    class Random {
    public:
        static f32 Float();
        static f32 Float( f32 min, f32 max );
        static i32 Int( i32 min, i32 max );
        static i32 Int( i32 max );

        static glm::vec2 Vec2( glm::vec2 min, glm::vec2 max );
    };

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