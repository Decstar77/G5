#pragma once

#include "../shared/atto_containers.h"
#include "../shared/atto_math.h"

namespace atto {
    constexpr i32 MAX_ENTITY_COUNT = 2048;
    constexpr i32 tickRate = 30; // 30 times a second
    constexpr f32 tickTime = 1.0f / tickRate;
    constexpr i32 tickCount = 2;


    struct VariableKeyFrame {
        glm::vec2 value;
        f32 t;
    };

    class VariableTimeline {
    public:
        GrowableList<VariableKeyFrame>  keyFrames;

        VariableKeyFrame    AddKeyFrame( glm::vec2 value, f32 t );
        void                RemoveKeyFramesPast( f32 t );
        bool                HasKeyFramePast( f32 t );
        glm::vec2           ValueForTime( f32 t );
        void                KeysFromTime( VariableKeyFrame ** left, VariableKeyFrame ** right, f32 t );
    };

    class PosTimeline {
    public:
        f32 time;
        FixedList<glm::vec2, 3> frames;
        glm::vec2           UpdateAndGet( f32 dt );
        bool                GetMovingDirection( glm::vec2 & dir );
        void                AddFrame( glm::vec2 frame );
    };
}


