#include "atto_math.h"
#include "atto_math_fixed.h"
#include "atto_containers.h"

namespace atto {

    class AsinLookup {
    public:
        static constexpr int TABLE_SIZE = 1000;
        static constexpr double STEP_SIZE = 2.0 / TABLE_SIZE;
        static constexpr FixedPoint STEP_SIZE_FP = CreateFixed((f32)STEP_SIZE);

        AsinLookup() {
            for (i32 i = 0; i <= TABLE_SIZE; ++i) {
                f64 x = -1.0 + i * STEP_SIZE;
                f64 r = asin(x);
                table.Add(CreateFixed((f32)r));
            }
        }

        FixedPoint operator[](FixedPoint x) const {
            if (x < CreateFixed(-1.0f) || x > CreateFixed(1.0f)) {
                INVALID_CODE_PATH;
            }

            i32 index = (i32)FixedToFloat((x + CreateFixed(1.0f)) / STEP_SIZE_FP);

            return table[index];
        }

    private:
        FixedList<FixedPoint, TABLE_SIZE + 1> table = {};
    };

    class AcosLookup {
    public:
        static constexpr int TABLE_SIZE = 1000;
        static constexpr double STEP_SIZE = 2.0 / TABLE_SIZE;
        static constexpr FixedPoint STEP_SIZE_FP = CreateFixed((f32)STEP_SIZE);

        AcosLookup() {
            for (i32 i = 0; i <= TABLE_SIZE; ++i) {
                f64 x = -1.0 + i * STEP_SIZE;
                f64 r = acos(x);
                table.Add( CreateFixed((f32)r) );
            }
        }

        FixedPoint operator[](FixedPoint x) const {
            if (x < CreateFixed(-1.0f) || x > CreateFixed(1.0f)) {
                INVALID_CODE_PATH;
            }

            i32 index = (i32)FixedToFloat((x + CreateFixed(1.0f)) / STEP_SIZE_FP);

            return table[index];
        }

    private:
        FixedList<FixedPoint, TABLE_SIZE + 1> table = {};
    };

    class AtanLookup {
    public:
        static constexpr int TABLE_SIZE = 1000;
        static constexpr double STEP_SIZE = 2.0 / TABLE_SIZE;
        static constexpr FixedPoint STEP_SIZE_FP = CreateFixed((f32)STEP_SIZE);

        AtanLookup() {
            for (i32 i = 0; i <= TABLE_SIZE; ++i) {
                f64 x = -1.0 + i * STEP_SIZE;
                f64 r = atan(x);
                table.Add(CreateFixed((f32)r));
            }
        }

        FixedPoint operator[](FixedPoint x) const {
            if (x < CreateFixed(-1.0f) || x > CreateFixed(1.0f)) {
                INVALID_CODE_PATH;
            }

            i32 index = (i32)FixedToFloat((x + CreateFixed(1.0f)) / STEP_SIZE_FP);

            return table[index];
        }

    private:
        FixedList<FixedPoint, TABLE_SIZE + 1> table = {};
    };

    static AcosLookup asinLookup = {};
    static AcosLookup acosLookup = {};
    static AcosLookup atanLookup = {};

    FixedPoint FPAcrSin(FixedPoint x) {
        return asinLookup[x];
    }
    
    FixedPoint FPAcrCos(FixedPoint x) {
        return acosLookup[x];
    }

    FixedPoint FPAcrTan(FixedPoint x) {
        INVALID_CODE_PATH // Untested
        return atanLookup[x];
    }

}

