#include "atto_math.h"

namespace atto {
    bool Circle::Intersects(const Circle& circle) {
        f32 distSqrd = glm::distance2(pos, circle.pos);
        f32 radSum = rad + circle.rad;
        return distSqrd < radSum* radSum;
    }

    bool Circle::Collision(const Circle& other, Manifold& manifold) const {
        glm::vec2 normal = other.pos - pos;
        f32 dist = glm::length(normal);
        f32 radSum = rad + other.rad;
        if (dist > radSum) {
            return false;
        }

        manifold.normal = glm::normalize(normal);
        manifold.penetration = radSum - dist;
        manifold.pointA = pos + manifold.normal * rad;
        manifold.pointB = other.pos - manifold.normal * other.rad;
        return true;
    }

    bool Circle::Contains(glm::vec2 c, f32 r, glm::vec2 point) {
        return glm::distance2(c, point) < r * r;
    }

    bool Circle::Contains(glm::vec2 point) const {
        return glm::distance2(this->pos, point) < rad * rad;
    }

    Circle Circle::Create(glm::vec2 pos, f32 rad) {
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
        return (min + max) * 0.5f;
    }

    glm::vec2 BoxBounds::GetSize() const {
        return max - min;
    }

    void BoxBounds::Translate(const glm::vec2& translation) {
        min += translation;
        max += translation;
    }

    void BoxBounds::CreateFromCenterSize(const glm::vec2& center, const glm::vec2& size) {
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

    bool BoxBounds::Intersects(const BoxBounds& other) const {
        return (max.x >= other.min.x && min.x <= other.max.x) &&
            (max.y >= other.min.y && min.y <= other.max.y);
    }

    bool BoxBounds::Intersects(const Circle& other) const {
        glm::vec2 closestPoint = glm::clamp(other.pos, min, max);
        f32 distSqrd = glm::distance2(other.pos, closestPoint);
        return distSqrd < other.rad* other.rad;
    }

    bool BoxBounds::Collision(const BoxBounds& other, Manifold& manifold) const {
        if (Intersects(other)) {
            f32 xOverlap = glm::min(max.x, other.max.x) - glm::max(min.x, other.min.x);
            f32 yOverlap = glm::min(max.y, other.max.y) - glm::max(min.y, other.min.y);

            if (xOverlap < yOverlap) {
                if (max.x > other.max.x) {
                    manifold.normal = glm::vec2(-1.0f, 0.0f);
                }
                else {
                    manifold.normal = glm::vec2(1.0f, 0.0f);
                }

                manifold.penetration = xOverlap;
            }
            else {
                if (max.y > other.max.y) {
                    manifold.normal = glm::vec2(0.0f, -1.0f);
                }
                else {
                    manifold.normal = glm::vec2(0.0f, 1.0f);
                }

                manifold.penetration = yOverlap;
            }

            return true;
        }

        return false;
    }

    bool BoxBounds::Contains(const glm::vec2& point) const {
        return (point.x >= min.x && point.x <= max.x) &&
            (point.y >= min.y && point.y <= max.y);
    }

    void BoxBounds::Expand(f32 mul) {
        glm::vec2 center = GetCenter();
        glm::vec2 size = GetSize();
        size *= mul;
        CreateFromCenterSize(center, size);
    }
}