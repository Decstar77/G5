#include "atto_sim_var.h"
#include "../shared/atto_logging.h"

namespace atto {
    VariableKeyFrame VariableTimeline::AddKeyFrame( glm::vec2 value, f32 t ) {
        VariableKeyFrame newKeyFrame;
        newKeyFrame.value = value;
        newKeyFrame.t = t;

        if( keyFrames.GetCount() == 0 ) {
            keyFrames.Add( newKeyFrame );
            return newKeyFrame;
        }

        if( t <= keyFrames[ 0 ].t ) {
            keyFrames.Insert( 0, newKeyFrame );
            return newKeyFrame;
        }

        for( i32 i = 0; i < keyFrames.GetCount() - 1; ++i ) {
            if( t >= keyFrames[ i ].t && t <= keyFrames[ i + 1 ].t ) {
                keyFrames.Insert( i + 1, newKeyFrame );
                return newKeyFrame;
            }
        }

        keyFrames.Add( newKeyFrame );

        return newKeyFrame;
    }

    void VariableTimeline::RemoveKeyFramesPast( f32 t ) {
        const i32 keyCount = keyFrames.GetCount();
        if( keyCount == 0 ) {
            return;
        }

        for( i32 i = keyCount - 1; i >= 0; --i ) {
            if( keyFrames[ i ].t <= t ) {
                keyFrames.SetCount( i + 1 );
                return;
            }
        }
    }

    bool VariableTimeline::HasKeyFramePast( f32 t ) {
        const i32 keyCount = keyFrames.GetCount();
        if( keyCount == 0 ) {
            return false;
        }

        if( keyFrames[ keyCount - 1 ].t <= t ) {
            return false;
        }

        return true;
    }

    glm::vec2 VariableTimeline::ValueForTime( f32 t ) {
        const i32 keyCount = keyFrames.GetCount();
        if( keyCount == 0 ) {
            ATTOWARN( "ValueForTime has no keyframes" );
            return glm::vec2( 0.0f, 0.0f );
        }

        if( t <= keyFrames[ 0 ].t ) {
            return keyFrames[ 0 ].value;
        }
        else if( t >= keyFrames[ keyCount - 1 ].t ) {
            return keyFrames[ keyCount - 1 ].value;
        }

        VariableKeyFrame * left = nullptr;
        VariableKeyFrame * right = nullptr;
        KeysFromTime( &left, &right, t );

        f32 deltaT = right->t - left->t;
        if( deltaT == 0.0 ) {
            return left->value;
        }

        f32 factor = ( t - left->t ) / deltaT;
        return left->value + factor * ( right->value - left->value );
    }

    void VariableTimeline::KeysFromTime( VariableKeyFrame ** left, VariableKeyFrame ** right, f32 t ) {
        *left = nullptr;
        *right = nullptr;

        const i32 keyCount = keyFrames.GetCount();
        for( i32 keyIndex = 0; keyIndex < keyCount; keyIndex++ ) {
            if( t >= keyFrames[ keyIndex ].t ) {
                *left = &keyFrames[ keyIndex ];
            }
            else if( t < keyFrames[ keyIndex ].t ) {
                *right = &keyFrames[ keyIndex ];
                break; // @NOTE: We can break here because the key array is assumed to be sorted.
            }
        }
    }

    glm::vec2 PosTimeline::UpdateAndGet( f32 dt ) {
        const i32 count = frames.GetCount();
        if( count == 1 ) {
            return frames[ 0 ];
        }
        else if( count == 2 ) {
            time += dt;
            f32 nt = glm::max( time / tickTime, 0.0f );
            glm::vec2 pos = glm::mix( frames[ 0 ], frames[ 1 ], glm::min( nt, 1.0f ) );
            if( nt > 1 ) {
                frames.RemoveIndex( 0 );
            }
            return pos;
        }
        else if( count == 3 ) {
            time += dt;
            f32 nt = glm::max( time / tickTime, 0.0f );
            glm::vec2 pos = glm::vec2( 0.0f );
            if( nt > 1 ) {
                nt -= 1.0f;
                time -= tickTime;
                pos = glm::mix( frames[ 1 ], frames[ 2 ], glm::min( nt, 1.0f ) );
                frames.RemoveIndex( 0 );
            }
            else {
                pos = glm::mix( frames[ 0 ], frames[ 1 ], glm::min( nt, 1.0f ) );
            }

            return pos;
        }

        INVALID_CODE_PATH;
        return glm::vec2( 0, 0 );
    }

    bool PosTimeline::GetMovingDirection( glm::vec2 & dir ) {
        const i32 count = frames.GetCount();
        if ( count == 1 ) {
            dir = glm::vec2( 0 );
            return false;
        } else if ( count == 2 ) {
            dir = frames[ 1 ] - frames[ 0 ];
            return true;
        } else if ( count == 3 ) {
            f32 nt = glm::max( time / tickTime, 0.0f );
            if( nt > 1 ) {
                dir = frames[ 2 ] - frames[ 1 ];
            }
            else {
                dir = frames[ 1 ] - frames[ 0 ];
            }
            return true;
        }

        INVALID_CODE_PATH;
        return false;
    }

    void PosTimeline::AddFrame( glm::vec2 frame ) {
        const i32 count = frames.GetCount();
        if( count == 0 ) {
            frames.Add( frame );
        }
        else if( count == 1 ) {
            time = -tickTime;
            frames.Add( frame );
        }
        else if( count == 2 ) {
            frames.Add( frame );
        }
        else if( count == 3 ) {
            frames.RemoveIndex( 0 );
            frames.Add( frame );
            time = 0;
        }
    }
}

