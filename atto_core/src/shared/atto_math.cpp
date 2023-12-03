#include "atto_math.h"

namespace atto {
    bool Circle::Intersects( const Circle & circle ) const {
        f32 distSqrd = glm::distance2( pos, circle.pos );
        f32 radSum = rad + circle.rad;
        return distSqrd < radSum * radSum;
    }

    bool Circle::Collision( const Circle & other, Manifold & manifold ) const {
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

    bool BoxBounds::Collision( const BoxBounds & other, Manifold & manifold ) const {
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

    bool BoxBounds::Collision( const Circle & circle, Manifold & manifold ) const {
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


    void Collider::Translate( const glm::vec2 & translation ) {
        switch( type ) {
            case COLLIDER_TYPE_CIRCLE: circle.pos += translation; return;
            case COLLIDER_TYPE_BOX: box.Translate( translation ); return;
        }
        Assert( false );
    }

    bool Collider::Contains( const glm::vec2 & point ) const {
        switch( type ) {
            case COLLIDER_TYPE_CIRCLE: return circle.Contains( point );
            case COLLIDER_TYPE_BOX: return box.Contains( point );
        }
        Assert( false );
        return false;
    }


    bool Collider::Intersects( const Collider & other ) const {
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


    bool Collider::Collision( const Collider & other, Manifold & manifold ) const {
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

}