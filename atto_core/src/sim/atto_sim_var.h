#pragma once

#include "../shared/atto_containers.h"
#include "../shared/atto_math.h"

namespace atto {
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

}


