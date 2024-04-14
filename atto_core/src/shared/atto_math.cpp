#include "atto_math.h"

#include "atto_reflection.h"

namespace atto {

    glm::vec2 ToVec2( const fp2 & v ) {
        return glm::vec2( static_cast<float>( v.x ), static_cast<float>( v.y ) );
    }

    fp2 ToFP2( const glm::vec2& v ) {
        return { fp( v.x ), fp( v.y ) };
    }

    fp FpAbs( fp f ) {
        return fpm::abs( f );
    }

    fp FpRound( fp f ) {
        return fpm::round( f );
    }

    fp FpSin( fp f ) {
        return fpm::sin( f );
    }

    fp FpCos( fp f ) {
        return fpm::cos( f );
    }

    fp FpTan( fp f ) {
        return fpm::tan( f );
    }

    fp FpASin( fp f ) {
        return fpm::asin( f );
    }
    
    fp FpACos( fp f ) {
        return fpm::acos( f );
    }

    fp FpATan2( fp y, fp x ) {
        return fpm::atan2( y, x );
    }

    fp FpMin(fp a, fp b) {
        return a < b ? a : b;
    }

    fp FpMax(fp a, fp b) {
        return a > b ? a : b;
    }

    fp FpClamp( fp f, fp min, fp max ) {
        return FpMax( min, FpMin( f, max ) );
    }

    fp FpLerp( fp a, fp b, fp t ) {
        return b * t + a * ( 1 - t );
    }

    fp2 FpAbs( fp2 v ) {
        return { FpAbs( v.x ), FpAbs( v.y ) };
    }

    fp FpLength( fp2 v ) {
        return fpm::sqrt( v.x * v.x + v.y * v.y );
    }

    fp FpLength2( fp2 v ) {
        return v.x * v.x + v.y * v.y;
    }

    fp FpDistance( fp2 v1, fp2 v2 ) {
        return FpLength( v1 - v2 );
    }

    fp FpDistance2( fp2 v1, fp2 v2 ) {
        return FpLength2( v1 - v2 );
    }

    fp2 FpNormalize( fp2 v ) {
        fp len = FpLength( v );
        if (len == fp( 0 ) ) {
            return { fp( 0 ), fp( 0 ) };
        }
        return { v.x / len, v.y / len };
    }

    fp2 FpTruncateLength( fp2 v, fp max ) {
        fp len = FpLength( v );
        if (len <= max ) {
            return v;
        }
        return FpNormalize( v ) * max;
    }

    fp2 FpLeftPerp( fp2 v ) {
        return { -v.y, v.x };
    }

    fp2 FpRotate( fp2 v, fp angle ) {
        fp2 result;
        fp c = FpCos( angle );
        fp s = FpSin( angle );

        result.x = v.x * c - v.y * s;
        result.y = v.x * s + v.y * c;
        return result;
    }

    fp2 operator+( const fp2 & a, const fp2 & b ) {
        return { a.x + b.x, a.y + b.y };
    }

    fp2 operator-( const fp2& a, const fp2& b ) {
        return { a.x - b.x, a.y - b.y };
    }

    fp2 operator-( const fp2 & a ) {
        return { -a.x, -a.y };
    }

    bool operator==( const fp2 & a, const fp2 & b ) {
        return a.x == b.x && a.y == b.y;
    }

    bool operator!=( const fp2 & a, const fp2 & b ) {
        return a.x != b.x || a.y != b.y;
    }

    fp2 operator*( const fp2 & a, const fp & b ) {
        return { a.x * b, a.y * b };
    }

    fp2 operator/( const fp2& a, const fp& b ) {
        return { a.x / b, a.y / b };
    }
    
    fp2 operator*( const fp & a, const fp2 & b ) {
        return { b.x * a, b.y * a };
    }

    fp2 operator/( const fp & a, const fp2 & b ) {
        return { b.x / a, b.y / a };
    }

    FpCircle FpCircleCreate( fp2 pos, fp rad ) {
        FpCircle c = {};
        c.rad = rad;
        c.pos = pos;
        return c;
    }

    bool FpCircleIntersects( FpCircle a, FpCircle b ) {
        fp distSqrd = FpDistance2( a.pos, b.pos );
        fp radSum = a.rad + b.rad;
        return distSqrd < radSum * radSum;
    }

    fp FpColliderSurfaceDistance( FpCircle a, FpCircle b ) {
        return FpDistance( a.pos, b.pos ) - a.rad - b.rad;
    }

    bool FpCircleCollision( FpCircle a, FpCircle b, FpManifold & manifold ) {
        if ( FpCircleIntersects( a, b ) == false ) {
            return false;
        }

        fp2 normal = a.pos - b.pos;
        fp dist = FpLength( normal );
        fp radSum = a.rad + b.rad;
        if( dist > radSum ) {
            return false;
        }

        manifold.normal = normal / dist;
        manifold.penetration = radSum - dist;
        manifold.pointA = a.pos + manifold.normal * a.rad;
        manifold.pointB = b.pos - manifold.normal * b.rad;
        return true;
    }

    bool FpCircleContains( FpCircle a, fp2 point ) {
        return FpDistance2( a.pos, point ) < a.rad * a.rad;
    }

    FpAxisBox FpAxisBoxCreateFromMinMax( fp2 min, fp2 max ) {
        FpAxisBox box = {};
        box.min = min;
        box.max = max;
        return box;
    }
    
    FpAxisBox FpAxisBoxCreateFromCenterSize( fp2 center, fp2 size ) {
        FpAxisBox box = {};
        box.min = center - size / Fp(2);
        box.max = center + size / Fp(2);
        return box;
    }

    fp FpAxisBoxGetWidth( FpAxisBox b ) {
        return b.max.x - b.min.x;
    }

    fp FpAxisBoxGetHeight( FpAxisBox b ) {
        return b.max.y - b.min.y;
    }

    fp2 FpAxisBoxGetCenter( FpAxisBox b ) {
        return ( b.min + b.max ) / Fp(2);
    }

    fp2 FpAxisBoxGetSize( FpAxisBox b ) {
        return b.max - b.min;
    }

    void FpAxisBoxTranslate( FpAxisBox * b, fp2 translation ) {
        b->min = b->min + translation;
        b->max = b->max + translation;
    }

    fp FpColliderSurfaceDistance( FpAxisBox a, FpAxisBox b ) {
        fp2 aCenter = FpAxisBoxGetCenter( a );
        fp2 bCenter = FpAxisBoxGetCenter( b );
        fp2 dist = FpAbs( aCenter - bCenter ) - ( FpAxisBoxGetSize( a ) + FpAxisBoxGetSize( b ) ) / Fp( 2 );
        return FpMax( dist.x, dist.y );
    }
    
    fp FpColliderSurfaceDistance( FpAxisBox a, FpCircle c ) {
        fp2 closestPoint = FpClosestPointBox( a, c.pos );
        fp dist = FpDistance( c.pos, closestPoint );
        return dist - c.rad;
    }

    bool FpAxisBoxIntersects( FpAxisBox a, FpAxisBox b ) {
        return ( a.max.x >= b.min.x && a.min.x <= b.max.x ) &&
            ( a.max.y >= b.min.y && a.min.y <= b.max.y );
    }
    
    bool FpAxisBoxIntersects( FpAxisBox b, FpCircle c ) {
        fp2 closestPoint = FpClosestPointBox( b, c.pos );
        fp distSqrd = FpDistance2( c.pos, closestPoint );
        return distSqrd < c.rad * c.rad;
    }

    fp2 FpClosestPointBox( FpAxisBox b, fp2 p ) {
        fp2 result = p;
        result.x = FpClamp( result.x, b.min.x, b.max.x );
        result.y = FpClamp( result.y, b.min.y, b.max.y );
        return result;
    }

    bool FpAxisBoxCollision( FpAxisBox a, FpAxisBox b, FpManifold & manifold ) {
        if( FpAxisBoxIntersects( a, b ) ) {
            fp xOverlap = FpMin( a.max.x, b.max.x ) - FpMax( a.min.x, b.min.x );
            fp yOverlap = FpMin( a.max.y, b.max.y ) - FpMax( a.min.y, b.min.y );

            if( xOverlap < yOverlap ) {
                if( a.max.x > b.max.x ) {
                    manifold.normal = Fp2( -1.0f, 0.0f );
                }
                else {
                    manifold.normal = Fp2( 1.0f, 0.0f );
                }

                manifold.penetration = xOverlap;
            }
            else {
                if( a.max.y > b.max.y ) {
                    manifold.normal = Fp2( 0.0f, -1.0f );
                }
                else {
                    manifold.normal = Fp2( 0.0f, 1.0f );
                }

                manifold.penetration = yOverlap;
            }

            return true;
        }

        return false;
    }

    bool FpAxisBoxContains( FpAxisBox a , fp2 p ) {
        return ( p.x >= a.min.x && p.x <= a.max.x ) &&
            ( p.y >= a.min.y && p.y <= a.max.y );
    }

    void FpColliderSetPos( FpCollider * a, fp2 p ) {
        switch ( a->type ) {
            case COLLIDER_TYPE_CIRCLE: a->circle.pos = p; return;
            case COLLIDER_TYPE_AXIS_BOX: a->box = FpAxisBoxCreateFromCenterSize( p, FpAxisBoxGetSize( a->box ) ); return;
        }
        INVALID_CODE_PATH;
    }

    fp FpColliderSurfaceDistance( FpCollider a, FpCollider b ) {
        if ( a.type == COLLIDER_TYPE_CIRCLE ) {
            if ( b.type == COLLIDER_TYPE_CIRCLE ) {
                return FpColliderSurfaceDistance( a.circle, b.circle );
            }
            else if ( b.type == COLLIDER_TYPE_AXIS_BOX ) {
                return FpColliderSurfaceDistance( b.box, a.circle );
            }
        }
        else if ( a.type == COLLIDER_TYPE_AXIS_BOX ) {
            if ( b.type == COLLIDER_TYPE_CIRCLE ) {
                return FpColliderSurfaceDistance( a.box, b.circle );
            }
            else if ( b.type == COLLIDER_TYPE_AXIS_BOX ) {
                return FpColliderSurfaceDistance( a.box, b.box );
            }
        }

        INVALID_CODE_PATH;
        return Fp( -1 );
    }

    bool FpColliderIntersects( FpCollider a, FpCircle b ) {
        switch ( a.type ) {
            case COLLIDER_TYPE_CIRCLE: return FpCircleIntersects( a.circle, b );
            case COLLIDER_TYPE_AXIS_BOX: return FpAxisBoxIntersects( a.box, b );
        }
        INVALID_CODE_PATH;
        return false;
    }

    bool FpColliderIntersects( FpCollider a, FpAxisBox b ) {
        switch ( a.type ) {
            case COLLIDER_TYPE_CIRCLE: return FpAxisBoxIntersects( b, a.circle );
            case COLLIDER_TYPE_AXIS_BOX: return FpAxisBoxIntersects( a.box, b );
        }
        INVALID_CODE_PATH;
        return false;
    }

    bool FpColliderIntersects( FpCollider a, FpCollider b ) {
        if ( a.type == COLLIDER_TYPE_CIRCLE ) {
            if ( b.type == COLLIDER_TYPE_CIRCLE ) {
                return FpCircleIntersects( a.circle, b.circle );
            }
            else if ( b.type == COLLIDER_TYPE_AXIS_BOX ) {
                return FpAxisBoxIntersects( b.box, a.circle );
            }
        }
        else if ( a.type == COLLIDER_TYPE_AXIS_BOX ) {
            if ( b.type == COLLIDER_TYPE_CIRCLE ) {
                return FpAxisBoxIntersects( a.box, b.circle );
            }
            else if ( b.type == COLLIDER_TYPE_AXIS_BOX ) {
                return FpAxisBoxIntersects( a.box, b.box );
            }
        }
        INVALID_CODE_PATH;
        return false;
    }

    bool Circle::Intersects( const Circle & circle ) const {
        f32 distSqrd = glm::distance2( pos, circle.pos );
        f32 radSum = rad + circle.rad;
        return distSqrd < radSum * radSum;
    }

    bool Circle::Collision( const Circle & other, Manifold2D & manifold ) const {
        glm::vec2 normal = other.pos - pos;
        f32 dist = glm::length( normal );
        f32 radSum = rad + other.rad;
        if( dist > radSum ) {
            return false;
        }

        manifold.normal = glm::normalize( normal );
        manifold.penetration = radSum - dist;
        manifold.pointA = pos + manifold.normal * rad;
        manifold.pointB = other.pos - manifold.normal * other.rad;
        return true;
    }

    bool Circle::Contains( glm::vec2 c, f32 r, glm::vec2 point ) {
        return glm::distance2( c, point ) < r * r;
    }

    bool Circle::Contains( glm::vec2 point ) const {
        return glm::distance2( this->pos, point ) < rad * rad;
    }

    Circle Circle::Create( glm::vec2 pos, f32 rad ) {
        Circle c = {};
        c.pos = pos;
        c.rad = rad;
        return c;
    }

    f32 BoxBounds2D::GetWidth() const {
        return max.x - min.x;
    }

    f32 BoxBounds2D::GetHeight() const {
        return max.y - min.y;
    }

    glm::vec2 BoxBounds2D::GetCenter() const {
        return ( min + max ) * 0.5f;
    }

    glm::vec2 BoxBounds2D::GetSize() const {
        return max - min;
    }

    void BoxBounds2D::Translate( const glm::vec2 & translation ) {
        min += translation;
        max += translation;
    }

    void BoxBounds2D::CreateFromCenterSize( const glm::vec2 & center, const glm::vec2 & size ) {
        max = center + size * 0.5f;
        min = center - size * 0.5f;
    }

    //bool RayCast::Box(const BoxBounds& b, const Ray2D& r, f32& t) {
    //    f32 tx1 = (b.min.x - r.origin.x) * (1.0f / r.direction.x);
    //    f32 tx2 = (b.max.x - r.origin.x) * (1.0f / r.direction.x);
    //
    //    f32 tmin = glm::min(tx1, tx2);
    //    f32 tmax = glm::max(tx1, tx2);
    //
    //    f32 ty1 = (b.min.y - r.origin.y) * (1.0f / r.direction.y);
    //    f32 ty2 = (b.max.y - r.origin.y) * (1.0f / r.direction.y);
    //
    //    tmin = glm::max(tmin, glm::min(ty1, ty2));
    //    tmax = glm::min(tmax, glm::max(ty1, ty2));
    //
    //    t = tmin;
    //
    //    return tmax >= tmin;
    //}

    bool BoxBounds2D::Intersects( const BoxBounds2D & other ) const {
        return ( max.x >= other.min.x && min.x <= other.max.x ) &&
            ( max.y >= other.min.y && min.y <= other.max.y );
    }

    bool BoxBounds2D::Intersects( const Circle & other ) const {
        glm::vec2 closestPoint = glm::clamp( other.pos, min, max );
        f32 distSqrd = glm::distance2( other.pos, closestPoint );
        return distSqrd < other.rad * other.rad;
    }

    bool BoxBounds2D::Collision( const BoxBounds2D & other, Manifold2D & manifold ) const {
        if( Intersects( other ) ) {
            f32 xOverlap = glm::min( max.x, other.max.x ) - glm::max( min.x, other.min.x );
            f32 yOverlap = glm::min( max.y, other.max.y ) - glm::max( min.y, other.min.y );

            if( xOverlap < yOverlap ) {
                if( max.x > other.max.x ) {
                    manifold.normal = glm::vec2( -1.0f, 0.0f );
                }
                else {
                    manifold.normal = glm::vec2( 1.0f, 0.0f );
                }

                manifold.penetration = xOverlap;
            }
            else {
                if( max.y > other.max.y ) {
                    manifold.normal = glm::vec2( 0.0f, -1.0f );
                }
                else {
                    manifold.normal = glm::vec2( 0.0f, 1.0f );
                }

                manifold.penetration = yOverlap;
            }

            return true;
        }

        return false;
    }

    bool BoxBounds2D::Collision( const Circle & circle, Manifold2D & manifold ) const {
          // Calculate the closest point on the box to the circle
        glm::vec2 closestPoint = glm::clamp( circle.pos, min, max );

        // Calculate the vector from the circle's center to the closest point on the box
        glm::vec2 toClosestPoint = closestPoint - circle.pos;

        // Calculate the distance between the circle's center and the closest point on the box
        float distance = glm::length( toClosestPoint );

        // Check if the circle and box overlap
        if( distance < circle.rad ) {
            // Calculate the penetration depth
            manifold.penetration = circle.rad - distance;

            // Calculate the normal vector pointing from the box to the circle
            manifold.normal = glm::normalize( toClosestPoint );

            // Calculate the contact points on the circle and box
            manifold.pointA = circle.pos - manifold.normal * circle.rad;
            manifold.pointB = closestPoint;

            return true; // Collision detected
        }

        return false; // No collision
    }

    bool BoxBounds2D::Contains( const glm::vec2 & point ) const {
        return ( point.x >= min.x && point.x <= max.x ) &&
            ( point.y >= min.y && point.y <= max.y );
    }

    void BoxBounds2D::Expand( f32 mul ) {
        glm::vec2 center = GetCenter();
        glm::vec2 size = GetSize();
        size *= mul;
        CreateFromCenterSize( center, size );
    }


    void Collider2D::Translate( const glm::vec2 & translation ) {
        switch( type ) {
            case COLLIDER_TYPE_CIRCLE: circle.pos += translation; return;
            case COLLIDER_TYPE_AXIS_BOX: box.Translate( translation ); return;
        }
        Assert( false );
    }

    bool Collider2D::Contains( const glm::vec2 & point ) const {
        switch( type ) {
            case COLLIDER_TYPE_CIRCLE: return circle.Contains( point );
            case COLLIDER_TYPE_AXIS_BOX: return box.Contains( point );
        }
        Assert( false );
        return false;
    }


    bool Collider2D::Intersects( const Collider2D & other ) const {
        switch( type ) {
            case COLLIDER_TYPE_CIRCLE:
            {
                switch( other.type ) {
                    case COLLIDER_TYPE_CIRCLE:
                    {
                        return circle.Intersects( other.circle );
                    }
                    case COLLIDER_TYPE_AXIS_BOX:
                    {
                        return other.box.Intersects( circle );
                    }
                }
            }

            case COLLIDER_TYPE_AXIS_BOX:
            {
                switch( other.type ) {
                    case COLLIDER_TYPE_CIRCLE:
                    {
                        return box.Intersects( other.circle );
                    }
                    case COLLIDER_TYPE_AXIS_BOX:
                    {
                        return box.Intersects( other.box );
                    }
                }
            }
        }

        Assert( false );
        return false;
    }

    bool Collider2D::Intersects( const BoxBounds2D & bounds ) const {
        switch( type ) {
            case COLLIDER_TYPE_CIRCLE:
            {
                return bounds.Intersects( circle );
            }

            case COLLIDER_TYPE_AXIS_BOX:
            {
                return bounds.Intersects( box );
            }
        }

        Assert( false );
        return false;
    }

    bool Collider2D::Collision( const Collider2D & other, Manifold2D & manifold ) const {
        switch( type ) {
            case COLLIDER_TYPE_CIRCLE:
            {
                switch( other.type ) {
                    case COLLIDER_TYPE_CIRCLE:
                    {
                        return circle.Collision( other.circle, manifold );
                    }
                    case COLLIDER_TYPE_AXIS_BOX:
                    {
                        return other.box.Collision( circle, manifold );
                    }
                }
            }

            case COLLIDER_TYPE_AXIS_BOX:
            {
                switch( other.type ) {
                    case COLLIDER_TYPE_CIRCLE:
                    {
                        bool r = box.Collision( other.circle, manifold );
                        manifold.Flip();
                        return r;
                    }
                    case COLLIDER_TYPE_AXIS_BOX:
                    {
                        return box.Collision( other.box, manifold );
                    }
                }
            }
        }

        Assert( false );
        return false;
    }

    bool Collider2D::Collision( const BoxBounds2D & box, Manifold2D & manifold ) const {
        switch( type ) {
            case COLLIDER_TYPE_CIRCLE: return this->box.Collision( circle, manifold );
            case COLLIDER_TYPE_AXIS_BOX: return this->box.Collision( box, manifold );
        }
        
        Assert( false );
        return false;
    }

    f32 BoxBounds::GetWidth() const {
        return max.x - min.x;
    }

    f32 BoxBounds::GetHeight() const {
        return max.y - min.y;
    }

    f32 BoxBounds::GetDepth() const {
        return max.z - min.z;
    }

    glm::vec3 BoxBounds::GetCenter() const {
        return ( min + max ) * 0.5f;
    }

    glm::vec3 BoxBounds::GetSize() const {
        return max - min;
    }

    bool BoxBounds::Contains( glm::vec3 point ) const {
        return ( point.x >= min.x && point.x <= max.x ) &&
            ( point.y >= min.y && point.y <= max.y ) &&
            ( point.z >= min.z && point.z <= max.z );
    }

    void BoxBounds::Translate( glm::vec3 translation ) {
        min += translation;
        max += translation;
    }

    void BoxBounds::Expand( glm::vec3 p ) {
        if( p.x < min.x ) {
            min.x = p.x;
        }
        if( p.y < min.y ) {
            min.y = p.y;
        }
        if( p.z < min.z ) {
            min.z = p.z;
        }

        if( p.x > max.x ) {
            max.x = p.x;
        }
        if( p.y > max.y ) {
            max.y = p.y;
        }
        if( p.z > max.z ) {
            max.z = p.z;
        }
    }

    void BoxBounds::CreateFromCenterSize( glm::vec3 center, glm::vec3 size ) {
        max = center + size * 0.5f;
        min = center - size * 0.5f;
    }

    inline bool IsPointInsideTriangle( glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, glm::vec3 point ) {
        // Compute barycentric coordinates
        glm::vec3 v0 = p2 - p1;
        glm::vec3 v1 = p3 - p1;
        glm::vec3 v2 = point - p1;

        f32 dot00 = glm::dot( v0, v0 );
        f32 dot01 = glm::dot( v0, v1 );
        f32 dot02 = glm::dot( v0, v2 );
        f32 dot11 = glm::dot( v1, v1 );
        f32 dot12 = glm::dot( v1, v2 );

        // Compute barycentric coordinates
        f32 invDenom = 1.0f / ( dot00 * dot11 - dot01 * dot01 );
        f32 u = ( dot11 * dot02 - dot01 * dot12 ) * invDenom;
        f32 v = ( dot00 * dot12 - dot01 * dot02 ) * invDenom;

        // Check if point is in triangle
        return ( u >= 0.0f ) && ( v >= 0.0f ) && ( u + v <= 1.0f );
    }

    glm::vec3 ClosestPoint_Sphere( glm::vec3 c, f32 r, glm::vec3 p ) {
        glm::vec3 d = p - c;
        glm::vec3 result = c + ( glm::normalize( d ) * r );
        return result;
    }

    glm::vec3 ClosestPoint_Box( glm::vec3 min, glm::vec3 max, glm::vec3 p ) {
        glm::vec3 result = p;
        result.x = glm::clamp( result.x, min.x, max.x );
        result.y = glm::clamp( result.y, min.y, max.y );
        result.z = glm::clamp( result.z, min.z, max.z );
        return result;
    }

    glm::vec3 ClosestPoint_Plane( glm::vec3 c, glm::vec3 n, glm::vec3 p ) {
        f32 dot = glm::dot( n, p );
        f32 d = glm::dot( n, c );
        f32 dist = ( dot - d );
        glm::vec3 result = p - dist * n;
        return result;
    }

    glm::vec3 ClosestPoint_LineSegment( glm::vec3 a, glm::vec3 b, glm::vec3 p ) {
        glm::vec3 l = b - a;
        f32 nume = glm::dot( p - a, l );
        f32 demon = glm::dot( l, l );
        f32 t = nume / demon;
        t = glm::clamp( t, 0.0f, 1.0f );
        glm::vec3 result = a + l * t;
        return result;
    }

    glm::vec3 ClosestPoint_Triangle( glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, glm::vec3 n, glm::vec3 p ) {
        glm::vec3 res = ClosestPoint_Plane( p1, n, p );
        if( IsPointInsideTriangle( p1, p2, p3, res ) == false ) {
            glm::vec3 a = ClosestPoint_LineSegment( p1, p2, p );
            glm::vec3 b = ClosestPoint_LineSegment( p2, p3, p );
            glm::vec3 c = ClosestPoint_LineSegment( p3, p1, p );

            f32 aa = glm::length2( p - a );
            f32 bb = glm::length2( p - b );
            f32 cc = glm::length2( p - c );

            if( aa <= bb && aa <= cc ) {
                res = a;
            }
            else if( bb <= aa && bb <= cc ) {
                res = b;
            }
            else // @NOTE: (cc <= aa && cc <= bb)
            {
                res = c;
            }
        }
        return res;
    }

    glm::vec3 ClosestPoint_Triangle( Collider tri, glm::vec3 p ) {
        Assert( tri.type == COLLIDER_TYPE_TRIANGLE );
        return ClosestPoint_Triangle( tri.tri.p1, tri.tri.p2, tri.tri.p3, tri.tri.n, p );
    }

    bool CollisionCheck_SphereVsSphere( glm::vec3 c1, f32 r1, glm::vec3 c2, f32 r2, Manifold & manifold ) {
        glm::vec3 d = c2 - c1;
        f32 d2 = glm::length2( d );
        f32 r = r1 + r2;
        bool result = d2 <= r * r;
        if( result ) {
            manifold.pointA = c1 + ( glm::normalize( d ) * r1 );
            manifold.pointB = c2 - ( glm::normalize( d ) * r2 );
            manifold.normal = glm::normalize( manifold.pointB - manifold.pointA );
            manifold.penetration = glm::distance( manifold.pointA, manifold.pointB );
        }
        return result;
    }

    bool CollisionCheck_SphereVsBox( glm::vec3 c, f32 r, glm::vec3 min, glm::vec3 max, Manifold & manifold ) {
        glm::vec3 close = ClosestPoint_Box( min, max, c );
        f32 d2 = glm::distance2( close, c );
        bool result = d2 <= r * r;
        if( result ) {
            manifold.pointB = close;
            manifold.normal = glm::normalize( c - close );
            manifold.pointA = c - manifold.normal * r;
            manifold.penetration = glm::distance( manifold.pointA, manifold.pointB );
        }
        return result;
    }

    bool CollisionCheck_SphereVsTri( glm::vec3 c, f32 r, glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, glm::vec3 n, Manifold & manifold ) {
        glm::vec3 close = ClosestPoint_Triangle( p1, p2, p3, n, c );
        f32 d2 = glm::distance2( close, c );
        bool result = d2 <= r * r;
        if( result ) {
            manifold.pointB = close;
            manifold.normal = glm::normalize( c - close );
            manifold.pointA = c - manifold.normal * r;
            manifold.penetration = glm::distance( manifold.pointA, manifold.pointB );
        }
        return result;
    }

    bool CollisionCheck_SphereVsPlane( Collider s, Collider p, Manifold & manifold ) {
        Assert( s.type == COLLIDER_TYPE_SPHERE );
        Assert( p.type == COLLIDER_TYPE_PLANE );
        glm::vec3 close = ClosestPoint_Plane( p.plane.c, p.plane.n, s.sphere.c );
        f32 d2 = glm::distance2( close, s.sphere.c );
        bool result = d2 <= s.sphere.r * s.sphere.r;
        if( result == true ) {
            manifold.pointB = close;
            manifold.normal = glm::normalize( s.sphere.c - close );
            manifold.pointA = s.sphere.c - manifold.normal * s.sphere.r;
            manifold.penetration = glm::distance( manifold.pointA, manifold.pointB );
        }

        return result;
    }

    bool CollisionCheck_SphereVsTri( Collider s, Collider t, Manifold & manifold ) {
        Assert( s.type == COLLIDER_TYPE_SPHERE );
        Assert( t.type == COLLIDER_TYPE_TRIANGLE );
        return CollisionCheck_SphereVsTri( s.sphere.c, s.sphere.r, t.tri.p1, t.tri.p2, t.tri.p3, t.tri.n, manifold );
    }

    void Collider::Translate( glm::vec3 p ) {
        switch( type ) {
            case COLLIDER_TYPE_NONE:
            {
            } break;
            case COLLIDER_TYPE_SPHERE:
            {
                sphere.c += p;
            } break;
            case COLLIDER_TYPE_AXIS_BOX:
            {
                box.min += p;
                box.max += p;
            }break;
            case COLLIDER_TYPE_PLANE: 
            {
                plane.c += p;
            } break;
            case COLLIDER_TYPE_TRIANGLE:
            {
                tri.p1 += p;
                tri.p2 += p;
                tri.p3 += p;
            } break;
            default: INVALID_CODE_PATH; break;
        }
    }

}