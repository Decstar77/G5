#pragma once

#include "atto_defines.h"

namespace atto {
    struct FixedPoint {
        i32 value;
    };

    constexpr inline FixedPoint IntToFixed(i32 i) {
        Assert(i < I16_MAX, "Int to big for fixed point number");

        FixedPoint fp = {};
        fp.value = i << 16;
        return fp;
    }

    constexpr inline FixedPoint FloatToFixed(f32 f) {
        FixedPoint fp = {};
        fp.value = i32(f * f32(1 << 16) + (f >= 0 ? 0.5 : 0.5));
        return fp;
    }

    constexpr inline f32 FixedToFloat(FixedPoint fp) {
        return f32(fp.value) / f32(1 << 16);
    }

    // @NOTE: This will simply truncate the value
    constexpr inline i32 FixedToInt(FixedPoint f) {
        return f.value >> 16;
    }

    constexpr inline FixedPoint CreateFixed(f32 i) {
        return FloatToFixed(i);
    }

    constexpr inline FixedPoint CreateFixed(i32 i) {
        return IntToFixed(i);
    }

    constexpr inline FixedPoint CreateFP(f32 i) {
        return FloatToFixed(i);
    }

    constexpr inline FixedPoint CreateFP(i32 i) {
        return IntToFixed(i);
    }

    constexpr inline FixedPoint CreateFPBits(i32 i) {
        FixedPoint v = {};
        v.value = i;
        return v;
    }

    constexpr inline FixedPoint operator+(FixedPoint a, FixedPoint b) {
        FixedPoint fp = {};
        fp.value = a.value + b.value;
        return fp;
    }

    constexpr inline FixedPoint operator-(FixedPoint a, FixedPoint b) {
        FixedPoint fp = {};
        fp.value = a.value - b.value;
        return fp;
    }

    constexpr inline FixedPoint operator*(FixedPoint a, FixedPoint b) {
        FixedPoint fp = {};
        fp.value = i32((i64(a.value) * i64(b.value)) >> 16);
        return fp;
    }

    constexpr inline FixedPoint operator/(FixedPoint a, FixedPoint b) {
        FixedPoint fp = {};
        fp.value = i32((i64(a.value) << 16) / i64(b.value));
        return fp;
    }

    constexpr inline FixedPoint& operator+=(FixedPoint& a, FixedPoint b) {
        a.value += b.value;
        return a;
    }
    
    constexpr inline FixedPoint& operator-=(FixedPoint& a, FixedPoint b) {
        a.value -= b.value;
        return a;
    }

    constexpr inline FixedPoint& operator*=(FixedPoint& a, FixedPoint b) {
        a.value = i32((i64(a.value) * i64(b.value)) >> 16);
        return a;
    }
    
    constexpr inline FixedPoint& operator/=(FixedPoint& a, FixedPoint b) {
        a.value = i32((i64(a.value) << 16) / i64(b.value));
        return a;
    }

    constexpr inline bool operator==(FixedPoint a, FixedPoint b) {
        return a.value == b.value;
    }

    constexpr inline bool operator!=(FixedPoint a, FixedPoint b) {
        return a.value != b.value;
    }

    constexpr inline bool operator<(FixedPoint a, FixedPoint b) {
        return a.value < b.value;
    }

    constexpr inline bool operator<=(FixedPoint a, FixedPoint b) {
        return a.value <= b.value;
    }

    constexpr inline bool operator>(FixedPoint a, FixedPoint b) {
        return a.value > b.value;
    }

    constexpr inline bool operator>=(FixedPoint a, FixedPoint b) {
        return a.value >= b.value;
    }

    constexpr FixedPoint FP_PI = FloatToFixed(3.14159265358979323846f);
    constexpr FixedPoint FP_PI2 = FloatToFixed(6.28318530717958647692f);
    constexpr FixedPoint FP_HALF_PI = FloatToFixed(1.57079632679489661923f);

    constexpr inline FixedPoint FPMax(FixedPoint a, FixedPoint b) {
        return a > b ? a : b;
    }

    constexpr inline FixedPoint FPMin(FixedPoint a, FixedPoint b) {
        return a < b ? a : b;
    }

    constexpr inline FixedPoint FPClamp(FixedPoint a, FixedPoint min, FixedPoint max) {
        return FPMax(min, FPMin(a, max));
    }

    constexpr inline FixedPoint FPClamp(FixedPoint a, f32 min, f32 max) {
        return FPClamp(a, FloatToFixed(min), FloatToFixed(max));
    }
    
    constexpr inline FixedPoint FPSign(FixedPoint a) {
        FixedPoint fp = a < IntToFixed(0) ? IntToFixed(-1) : IntToFixed(1);
        return fp;
    }

    constexpr inline FixedPoint FPModF(FixedPoint a, FixedPoint b) {
        FixedPoint fp = {};
        fp.value = a.value % b.value;
        return fp;
    }

    // Thank to these legends https://github.com/chmike/fpsqrt/blob/master/fpsqrt.c
    constexpr inline FixedPoint FPSqrt(FixedPoint fp) {
        f32 test = FixedToFloat(fp);
        if (test < 0.0f) {
            INVALID_CODE_PATH;
            return {};
        }

        i32 v = fp.value;

        if (v == 0) {
            return {};
        }

        u32 r = v;
        u32 b = 0x40000000;
        u32 q = 0;

        while (b > 0x40) {
            u32 t = q + b;
            if (r >= t)
            {
                r -= t;
                q = t + b;
            }
            r <<= 1;
            b >>= 1;
        }

        FixedPoint result = {};
        result.value = q >> 8;

        return result;
    }

    constexpr inline FixedPoint FPSin(FixedPoint x) {
        // This sine uses a fifth-order curve-fitting approximation originally
        // described by Jasper on coranac.com which has a worst-case
        // relative error of 0.07% (over [-pi:pi]).

        // Turn x from [0..2*PI] domain into [0..4] domain
        x = FPModF(x, FP_PI2);
        x = x / FP_HALF_PI;

        // Take x modulo one rotation, so [-4..+4].
        if (x < IntToFixed(0)) {
            x = x + IntToFixed(4);
        }

        int sign = +1;
        if (x > IntToFixed(2)) {
            // Reduce domain to [0..2].
            sign = -1;
            x = x - IntToFixed(2);
        }

        if (x > IntToFixed(1)) {
            // Reduce domain to [0..1].
            x = IntToFixed(2) - x;
        }

        const FixedPoint x2 = x * x;
        
        return IntToFixed(sign) * x * (FP_PI - x2 * (FP_PI2 - IntToFixed(5) - x2 * (FP_PI - IntToFixed(3)))) / IntToFixed(2);
    }

    constexpr inline FixedPoint FPCos(FixedPoint x) {
        return FPSin(x + FP_HALF_PI);
    }

    constexpr inline FixedPoint FPTan(FixedPoint x) {
        return FPSin(x) / FPCos(x);
    }

    FixedPoint FPAcrSin(FixedPoint x);
    FixedPoint FPAcrCos(FixedPoint x);
    FixedPoint FPAcrTan(FixedPoint x);

    constexpr void Testing() {
        constexpr FixedPoint a = FloatToFixed(5.6f);
        constexpr FixedPoint b = FloatToFixed(2.7f);
        constexpr FixedPoint c = FloatToFixed(8);
        constexpr FixedPoint big = FloatToFixed(10000.0f);
        
        constexpr FixedPoint aa = a * b;
        constexpr FixedPoint bb = a / b;

        constexpr FixedPoint ee = FPSqrt(a);

        constexpr FixedPoint ii = IntToFixed(30324);
        
        constexpr FixedPoint s = FPSin(FP_PI2 + FP_HALF_PI);

        constexpr f32 r = FixedToFloat(s);
    }

    struct FixedVec2 {
        FixedPoint x;
        FixedPoint y;
    };

    constexpr inline FixedVec2 CreateFPVec2(FixedPoint x, FixedPoint y) {
        FixedVec2 v = {};
        v.x = x;
        v.y = y;
        return v;
    }

    constexpr inline FixedVec2 CreateFPVec2(f32 x, f32 y) {
        FixedVec2 v = {};
        v.x = FloatToFixed(x);
        v.y = FloatToFixed(y);
        return v;
    }

    constexpr inline FixedVec2 CreateFPVec2(i32 x, i32 y) {
        FixedVec2 v = {};
        v.x = FloatToFixed(f32(x));
        v.y = FloatToFixed(f32(y));
        return v;
    }

    constexpr inline FixedPoint FPDot(FixedVec2 a, FixedVec2 b) {
        return a.x * b.x + a.y * b.y;
    }

    constexpr inline FixedPoint FPCrossScalar(FixedVec2 a, FixedVec2 b) {
        return a.x * b.y - a.y * b.x;
    }

    constexpr inline FixedVec2 FPPerp(FixedVec2 a) {
        return CreateFPVec2(a.y, a.x * FloatToFixed(-1.0f));
    }

    constexpr inline FixedPoint FPLength(FixedVec2 v) {
        FixedPoint d = FPDot(v, v);
        float test1 = FixedToFloat(v.x);
        float test2 = FixedToFloat(v.y);
        float t = test1 * test1 + test2 * test2;
        FixedPoint tt = FloatToFixed(t);
        return FPSqrt(d);
    }

    constexpr inline FixedPoint FPLength2(FixedVec2 v) {
        return FPDot(v, v);
    }

    constexpr inline FixedPoint FPDistance2(FixedVec2 a, FixedVec2 b) {
        FixedVec2 diff = { a.x - b.x, a.y - b.y };
        return FPLength2(diff);
    }

    constexpr inline FixedPoint FPDistance(FixedVec2 a, FixedVec2 b) {
        FixedVec2 diff = { a.x - b.x, a.y - b.y };
        return FPLength(diff);
    }

    constexpr inline FixedVec2 FPClampLength(FixedVec2 a, FixedPoint l) {
        FixedPoint d = FPLength(a);
        if (d > l) {
            FixedPoint f = l / d;
            a.x = a.x * f;
            a.y = a.y * f;
        }
        return a;
    }

    constexpr inline FixedVec2 FPNormalize(FixedVec2 a) {
        FixedPoint d = FPLength(a);
        if (d > FloatToFixed(0.0f)) {
            a.x = a.x / d;
            a.y = a.y / d;
        }
        return a;
    }

    inline FixedPoint FPAngle(FixedVec2 a, FixedVec2 b) {
        FixedPoint d = FPClamp(FPDot(a, b), -1.0f, 1.0f);
        FixedPoint ang = FPAcrCos(d);
        return ang;
    }

    inline FixedVec2 FPRotate(FixedVec2 a, FixedPoint angle) {
        FixedPoint s = FPSin(angle);
        FixedPoint c = FPCos(angle);
        FixedVec2 v = {};
        v.x = a.x * c - a.y * s;
        v.y = a.x * s + a.y * c;
        return v;
    }

    constexpr inline bool operator==(FixedVec2 a, FixedVec2 b) {
        return a.x == b.x && a.y == b.y;
    }

    constexpr inline bool operator!=(FixedVec2 a, FixedVec2 b) {
        return !(a == b);
    }

    constexpr inline FixedVec2 operator+(FixedVec2 a, FixedVec2 b) {
        FixedVec2 v = {};
        v.x = a.x + b.x;
        v.y = a.y + b.y;
        return v;
    }

    constexpr inline FixedVec2 operator-(FixedVec2 a, FixedVec2 b) {
        FixedVec2 v = {};
        v.x = a.x - b.x;
        v.y = a.y - b.y;
        return v;
    }

    constexpr inline FixedVec2 operator*(FixedVec2 a, FixedPoint b) {
        FixedVec2 v = {};
        v.x = a.x * b;
        v.y = a.y * b;
        return v;
    }

    constexpr inline FixedVec2 operator/(FixedVec2 a, FixedPoint b) {
        FixedVec2 v = {};
        v.x = a.x / b;
        v.y = a.y / b;
        return v;
    }

    constexpr inline FixedVec2 operator*(FixedPoint a, FixedVec2 b) {
        FixedVec2 v = {};
        v.x = a * b.x;
        v.y = a * b.y;
        return v;
    }

    constexpr inline FixedVec2& operator+=(FixedVec2& a, FixedVec2 b) {
        a.x += b.x;
        a.y += b.y;
        return a;
    }

    constexpr inline FixedVec2& operator-=(FixedVec2& a, FixedVec2 b) {
        a.x -= b.x;
        a.y -= b.y;
        return a;
    }

    typedef FixedPoint fp;
    typedef FixedVec2 fpv2;

#define FP_MAX CreateFPBits(2147483647)
#define FP_MIN CreateFPBits(-2147483648)

    struct FPCircle {
        FixedVec2 center;
        FixedPoint radius;

        inline FPCircle() {
            center = {};
            radius = CreateFixed(0);
        }

        inline FPCircle(FixedVec2 c, FixedPoint r) {
            center = c;
            radius = r;
        }

        inline bool Contains(FixedVec2 p) {
            FixedPoint d = FPDistance2(center, p);
            return d <= radius * radius;
        }

        inline bool Intersects(FPCircle other) const {
            FixedPoint d = FPDistance2(center, other.center);
            FixedPoint r = radius + other.radius;
            return d <= r * r;
        }
        
        inline bool Intersects(FPCircle other, fpv2 &normal, fp &pen) const {
            FixedPoint d = FPDistance2(center, other.center);
            FixedPoint r = radius + other.radius;
            if (d <= r * r) {
                FixedPoint dist = FPSqrt(d);
                if (dist == CreateFixed(0)) {
                    normal = CreateFPVec2(1.0f, 0.0f);
                    pen = radius;
                }
                else {
                    normal = (other.center - center) / dist;
                    pen = r - dist;
                }
                return true;
            }
            return false;
        }
    };

    
    struct FPBoxBounds {
        FixedVec2 min;
        FixedVec2 max;

        inline FPBoxBounds() {
            min = {};
            max = {};
        }

        inline FPBoxBounds(FixedVec2 min, FixedVec2 max) {
            this->min = min;
            this->max = max;
        }

        inline bool Contains(FixedVec2 p) {
            return p.x >= min.x && p.x <= max.x && p.y >= min.y && p.y <= max.y;
        }

        inline fpv2 GetCenter() {
            return (min + max) / CreateFixed(2);
        }

        inline bool Intersects(FPCircle c) {
            FixedVec2 closest = {};
            closest.x = FPClamp(c.center.x, min.x, max.x);
            closest.y = FPClamp(c.center.y, min.y, max.y);
            FixedPoint d = FPDistance2(c.center, closest);
            return d <= c.radius * c.radius;
        }

        inline bool Intersects(FPCircle c, fpv2& normal, fp& pen) {
            FixedVec2 closest = {};
            closest.x = FPClamp(c.center.x, min.x, max.x);
            closest.y = FPClamp(c.center.y, min.y, max.y);
            FixedPoint d = FPDistance2(c.center, closest);
            if (d <= c.radius * c.radius) {
                FixedPoint dist = FPSqrt(d);
                if (dist == CreateFixed(0)) {
                    normal = CreateFPVec2(1.0f, 0.0f);
                    pen = c.radius;
                }
                else {
                    fp dir = FPSign(FPDot(GetCenter() - closest, c.center - closest)) * CreateFixed(-1);
                    normal = dir * (c.center - closest) / dist;
                    pen = c.radius - dist;
                }
                return true;
            }
            return false;
        }
    };

}
