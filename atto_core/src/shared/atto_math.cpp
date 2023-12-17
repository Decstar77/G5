#include "atto_math.h"

namespace atto {
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

    f32 BoxBounds::GetWidth() const {
        return max.x - min.x;
    }

    f32 BoxBounds::GetHeight() const {
        return max.y - min.y;
    }

    glm::vec2 BoxBounds::GetCenter() const {
        return ( min + max ) * 0.5f;
    }

    glm::vec2 BoxBounds::GetSize() const {
        return max - min;
    }

    void BoxBounds::Translate( const glm::vec2 & translation ) {
        min += translation;
        max += translation;
    }

    void BoxBounds::CreateFromCenterSize( const glm::vec2 & center, const glm::vec2 & size ) {
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

    bool BoxBounds::Intersects( const BoxBounds & other ) const {
        return ( max.x >= other.min.x && min.x <= other.max.x ) &&
            ( max.y >= other.min.y && min.y <= other.max.y );
    }

    bool BoxBounds::Intersects( const Circle & other ) const {
        glm::vec2 closestPoint = glm::clamp( other.pos, min, max );
        f32 distSqrd = glm::distance2( other.pos, closestPoint );
        return distSqrd < other.rad * other.rad;
    }

    bool BoxBounds::Collision( const BoxBounds & other, Manifold2D & manifold ) const {
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

    bool BoxBounds::Collision( const Circle & circle, Manifold2D & manifold ) const {
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

    bool BoxBounds::Contains( const glm::vec2 & point ) const {
        return ( point.x >= min.x && point.x <= max.x ) &&
            ( point.y >= min.y && point.y <= max.y );
    }

    void BoxBounds::Expand( f32 mul ) {
        glm::vec2 center = GetCenter();
        glm::vec2 size = GetSize();
        size *= mul;
        CreateFromCenterSize( center, size );
    }


    void Collider2D::Translate( const glm::vec2 & translation ) {
        switch( type ) {
            case COLLIDER_TYPE_CIRCLE: circle.pos += translation; return;
            case COLLIDER_TYPE_BOX: box.Translate( translation ); return;
        }
        Assert( false );
    }

    bool Collider2D::Contains( const glm::vec2 & point ) const {
        switch( type ) {
            case COLLIDER_TYPE_CIRCLE: return circle.Contains( point );
            case COLLIDER_TYPE_BOX: return box.Contains( point );
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
                    case COLLIDER_TYPE_BOX:
                    {
                        return other.box.Intersects( circle );
                    }
                }
            }

            case COLLIDER_TYPE_BOX:
            {
                switch( other.type ) {
                    case COLLIDER_TYPE_CIRCLE:
                    {
                        return box.Intersects( other.circle );
                    }
                    case COLLIDER_TYPE_BOX:
                    {
                        return box.Intersects( other.box );
                    }
                }
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
                    case COLLIDER_TYPE_BOX:
                    {
                        return other.box.Collision( circle, manifold );
                    }
                }
            }

            case COLLIDER_TYPE_BOX:
            {
                switch( other.type ) {
                    case COLLIDER_TYPE_CIRCLE:
                    {
                        bool r = box.Collision( other.circle, manifold );
                        manifold.Flip();
                        return r;
                    }
                    case COLLIDER_TYPE_BOX:
                    {
                        return box.Collision( other.box, manifold );
                    }
                }
            }
        }

        Assert( false );
        return false;
    }

    inline bool IsPointInsideTriangle( Collider triangle, glm::vec3 point ) {
        glm::vec3 a = triangle.tri.p1 - point;
        glm::vec3 b = triangle.tri.p2 - point;
        glm::vec3 c = triangle.tri.p3 - point;

        glm::vec3 u = glm::cross( b, c );
        glm::vec3 v = glm::cross( c, a );
        glm::vec3 w = glm::cross( a, b );

        f32 d = glm::dot( u, v );
        f32 e = glm::dot( u, w );

        bool result = ( d >= 0.0f && e >= 0.0f );

        return result;
    }

    glm::vec3 ClosestPoint_Sphere( glm::vec3 c, f32 r, glm::vec3 p ) {
        glm::vec3 d = p - c;
        glm::vec3 result = c + ( glm::normalize( d ) * r );
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

    glm::vec3 ClosestPoint_Triangle( Collider tri, glm::vec3 p ) {
        glm::vec3 res = ClosestPoint_Plane( tri.tri.p1, tri.tri.n, p );
        if( IsPointInsideTriangle( tri, res ) == false ) {
            glm::vec3 a = ClosestPoint_LineSegment( tri.tri.p1, tri.tri.p2, p );
            glm::vec3 b = ClosestPoint_LineSegment( tri.tri.p2, tri.tri.p3, p );
            glm::vec3 c = ClosestPoint_LineSegment( tri.tri.p3, tri.tri.p1, p );

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

    bool CollisionCheck_SphereVsPlane( Collider s, Collider p, Manifold & manifold ) {
        Assert( s.type == COLLIDER_TYPE_SHPERE );
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
        Assert( s.type == COLLIDER_TYPE_SHPERE );
        Assert( t.type == COLLIDER_TYPE_TRIANGLE );
        glm::vec3 close = ClosestPoint_Triangle( t, s.sphere.c );
        f32 d2 = glm::distance2( close, s.sphere.c );
        bool result = d2 <= s.sphere.r * s.sphere.r;
        return result;
    }

}