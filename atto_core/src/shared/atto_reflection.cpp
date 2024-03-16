#include "atto_reflection.h"

namespace atto {
    nlohmann::json JSON_Write( bool v ) {
        return nlohmann::json( v );
    }

    nlohmann::json JSON_Write( i32 v ) {
        return nlohmann::json( v );
    }

    nlohmann::json JSON_Write( i64 v ) {
        return nlohmann::json( v );
    }

    nlohmann::json JSON_Write( u8 v ) {
        return nlohmann::json( v );
    }

    nlohmann::json JSON_Write( u32 v ) {
        return nlohmann::json( v );
    }

    nlohmann::json JSON_Write( u64 v ) {
        return nlohmann::json( v );
    }

    nlohmann::json JSON_Write( f32 v ) {
        return nlohmann::json( v );
    }

    nlohmann::json JSON_Write( f64 v ) {
        return nlohmann::json( v );
    }

    nlohmann::json JSON_Write( const SmallString & v ) {
        return nlohmann::json( v.GetCStr() );
    }

    nlohmann::json JSON_Write( const LargeString & v ) {
        return nlohmann::json( v.GetCStr() );
    }

    nlohmann::json JSON_Write( glm::vec2 v ) {
        nlohmann::json j = {};
        j[ "x" ] = v.x;
        j[ "y" ] = v.y;
        return j;
    }

    nlohmann::json JSON_Write( glm::vec3 v ) {
        nlohmann::json j = {};
        j[ "x" ] = v.x;
        j[ "y" ] = v.y;
        j[ "z" ] = v.z;
        return j;
    }

    nlohmann::json JSON_Write( glm::vec4 v ) {
        nlohmann::json j = {};
        j[ "x" ] = v.x;
        j[ "y" ] = v.y;
        j[ "z" ] = v.z;
        j[ "w" ] = v.w;
        return j;
    }

    nlohmann::json JSON_Write( glm::mat2 v ) {
        nlohmann::json j = {};
        j[ "col1" ] = JSON_Write( v[ 0 ] );
        j[ "col2" ] = JSON_Write( v[ 1 ] );
        return j;
    }

    nlohmann::json JSON_Write( glm::mat3 v ) {
        nlohmann::json j = {};
        j[ "col1" ] = JSON_Write( v[ 0 ] );
        j[ "col2" ] = JSON_Write( v[ 1 ] );
        j[ "col3" ] = JSON_Write( v[ 2 ] );
        return j;
    }

    nlohmann::json JSON_Write( glm::mat4 v ) {
        nlohmann::json j = {};
        j[ "col1" ] = JSON_Write( v[ 0 ] );
        j[ "col2" ] = JSON_Write( v[ 1 ] );
        j[ "col3" ] = JSON_Write( v[ 2 ] );
        j[ "col4" ] = JSON_Write( v[ 3 ] );
        return j;
    }

    void JSON_Read( const nlohmann::json & j, u8 & o ) {
        o = j.get<u8>();
    }

    void JSON_Read( const nlohmann::json & j, bool & o ) {
        o = j.get<bool>();
    }

    void JSON_Read( const nlohmann::json & j, i32 & o ) {
        o = j.get<i32>();
    }

    void JSON_Read( const nlohmann::json & j, i64 & o ) {
        o = j.get<i64>();
    }

    void JSON_Read( const nlohmann::json & j, u32 & o ) {
        o = j.get<u32>();
    }

    void JSON_Read( const nlohmann::json & j, u64 & o ) {
        o = j.get<u64>();
    }

    void JSON_Read( const nlohmann::json & j, f32 & o ) {
        o = j.get<f32>();
    }

    void JSON_Read( const nlohmann::json & j, f64 & o ) {
        o = j.get<f64>();
    }

    void JSON_Read( const nlohmann::json & j, SmallString & o ) {
        o = SmallString::FromLiteral( j.get<std::string>().c_str() );
    }

    void JSON_Read( const nlohmann::json & j, LargeString & o ) {
        o = LargeString::FromLiteral( j.get<std::string>().c_str() );
    }

    void JSON_Read( const nlohmann::json & j, glm::vec2 & o ) {
        o = glm::vec2( j.at( "x" ).get<float>(), j.at( "y" ).get<float>() );
    }

    void JSON_Read( const nlohmann::json & j, glm::vec3 & o ) {
        o = glm::vec3( j.at( "x" ).get<float>(), j.at( "y" ).get<float>(), j.at( "z" ).get<float>() );
    }

    void JSON_Read( const nlohmann::json & j, glm::vec4 & o ) {
        o = glm::vec4( j.at( "x" ).get<float>(), j.at( "y" ).get<float>(), j.at( "z" ).get<float>(), j.at( "w" ).get<float>() );
    }

    void JSON_Read( const nlohmann::json & j, glm::mat2 & o ) {
        glm::vec2 col1;
        glm::vec2 col2;
        JSON_Read( j.at( "col1" ), col1 );
        JSON_Read( j.at( "col2" ), col2 );
        o = glm::mat2( col1, col2 );
    }

    void JSON_Read( const nlohmann::json & j, glm::mat3 & o ) {
        glm::vec3 col1;
        glm::vec3 col2;
        glm::vec3 col3;
        JSON_Read( j.at( "col1" ), col1 );
        JSON_Read( j.at( "col2" ), col2 );
        JSON_Read( j.at( "col3" ), col3 );
        o = glm::mat3( col1, col2, col3 );
    }

    void JSON_Read( const nlohmann::json & j, glm::mat4 & o ) {
        glm::vec4 col1;
        glm::vec4 col2;
        glm::vec4 col3;
        glm::vec4 col4;
        JSON_Read( j.at( "col1" ), col1 );
        JSON_Read( j.at( "col2" ), col2 );
        JSON_Read( j.at( "col3" ), col3 );
        JSON_Read( j.at( "col4" ), col4 );
        o = glm::mat4( col1, col2, col3, col4 );
    }

    struct TypeDescriptor_I32 : TypeDescriptor {
        TypeDescriptor_I32() {
            name = "i32";
            size = sizeof( i32 );
        }

        virtual nlohmann::json JSON_Write( const void * obj ) override {
            return nlohmann::json( *(i32 *)obj );
        }

        virtual void JSON_Read( const nlohmann::json & j, const void * obj ) override {
            *(i32 *)obj = j.get<i32>();
        }

        virtual void Imgui_Draw( const void * obj, const char * memberName ) override {
            ImGui::InputInt( memberName, (i32 *)obj );
        }
    };

    template <>
    TypeDescriptor * GetPrimitiveDescriptor<int>() {
        static TypeDescriptor_I32 typeDesc;
        return &typeDesc;
    }

    struct TypeDescriptor_U32 : TypeDescriptor {
        TypeDescriptor_U32() {
            name = "u32";
            size = sizeof( u32 );
        }

        virtual nlohmann::json JSON_Write( const void * obj ) override {
            return nlohmann::json( *(u32 *)obj );
        }

        virtual void JSON_Read( const nlohmann::json & j, const void * obj ) override {
            *(u32 *)obj = j.get<u32>();
        }

        virtual void Imgui_Draw( const void * obj, const char * memberName ) override {
            ImGui::InputScalar( memberName, ImGuiDataType_U32, (u32 *)obj );
        }
    };

    template <>
    TypeDescriptor * GetPrimitiveDescriptor<unsigned int>() {
        static TypeDescriptor_U32 typeDesc;
        return &typeDesc;
    }

    struct TypeDescriptor_I64 : TypeDescriptor {
        TypeDescriptor_I64() {
            name = "i64";
            size = sizeof( i64 );
        }

        virtual nlohmann::json JSON_Write( const void * obj ) override {
            return nlohmann::json( *(i64 *)obj );
        }

        virtual void JSON_Read( const nlohmann::json & j, const void * obj ) override {
            *(i64 *)obj = j.get<i64>();
        }

        virtual void Imgui_Draw( const void * obj, const char * memberName ) override {
            ImGui::InputScalar( memberName, ImGuiDataType_S64, (i64 *)obj );
        }
    };

    template <>
    TypeDescriptor * GetPrimitiveDescriptor<i64>() {
        static TypeDescriptor_I64 typeDesc;
        return &typeDesc;
    }

    struct TypeDescriptor_U64 : TypeDescriptor {
        TypeDescriptor_U64() {
            name = "u64";
            size = sizeof( u64 );
        }

        virtual nlohmann::json JSON_Write( const void * obj ) override {
            return nlohmann::json( *(u64 *)obj );
        }

        virtual void JSON_Read( const nlohmann::json & j, const void * obj ) override {
            *(u64 *)obj = j.get<u64>();
        }

        virtual void Imgui_Draw( const void * obj, const char * memberName ) override {
            throw std::logic_error( "The method or operation is not implemented." );
        }
    };

    template <>
    TypeDescriptor * GetPrimitiveDescriptor<u64>() {
        static TypeDescriptor_U64 typeDesc;
        return &typeDesc;
    }

    struct TypeDescriptor_F32 : TypeDescriptor {
        TypeDescriptor_F32() {
            name = "f32";
            size = sizeof( f32 );
        }

        virtual nlohmann::json JSON_Write( const void * obj ) override {
            return nlohmann::json( *(f32 *)obj );
        }

        virtual void JSON_Read( const nlohmann::json & j, const void * obj ) override {
            *(f32 *)obj = j.get<f32>();
        }

        virtual void Imgui_Draw( const void * obj, const char * memberName ) override {
            ImGui::InputFloat( memberName, (float *)obj );
        }
    };

    template <>
    TypeDescriptor * GetPrimitiveDescriptor<f32>() {
        static TypeDescriptor_F32 typeDesc;
        return &typeDesc;
    }

    struct TypeDescriptor_F64 : TypeDescriptor {
        TypeDescriptor_F64() {
            name = "f64";
            size = sizeof( f64 );
        }

        virtual nlohmann::json JSON_Write( const void * obj ) override {
            return nlohmann::json( *(f64 *)obj );
        }

        virtual void JSON_Read( const nlohmann::json & j, const void * obj ) override {
            *(f64 *)obj = j.get<f64>();
        }

        virtual void Imgui_Draw( const void * obj, const char * memberName ) override {
            ImGui::InputDouble( memberName, (double *)obj );
        }
    };

    template <>
    TypeDescriptor * GetPrimitiveDescriptor<f64>() {
        static TypeDescriptor_F64 typeDesc;
        return &typeDesc;
    }

    struct TypeDescriptor_Bool : TypeDescriptor {
        TypeDescriptor_Bool() {
            name = "bool";
            size = sizeof( bool );
        }

        virtual nlohmann::json JSON_Write( const void * obj ) override {
            return nlohmann::json( *(bool *)obj );
        }

        virtual void JSON_Read( const nlohmann::json & j, const void * obj ) override {
            *(bool *)obj = j.get<bool>();
        }

        virtual void Imgui_Draw( const void * obj, const char * memberName ) override {
            ImGui::Checkbox( memberName, (bool *)obj );
        }
    };

    template <>
    TypeDescriptor * GetPrimitiveDescriptor<bool>() {
        static TypeDescriptor_Bool typeDesc;
        return &typeDesc;
    }

    struct TypeDescriptor_Vec2 : TypeDescriptor {
        TypeDescriptor_Vec2() {
            name = "vec2";
            size = sizeof( glm::vec2 );
        }

        virtual nlohmann::json JSON_Write( const void * obj ) override {
            return atto::JSON_Write( *(glm::vec2 *)obj );
        }

        virtual void JSON_Read( const nlohmann::json & j, const void * obj ) override {
            atto::JSON_Read( j, *(glm::vec2 *)obj );
        }

        virtual void Imgui_Draw( const void * obj, const char * memberName ) override {
            ImGui::InputFloat2( memberName, (float *)obj );
        }
    };

    template <>
    TypeDescriptor * GetPrimitiveDescriptor<glm::vec2>() {
        static TypeDescriptor_Vec2 typeDesc;
        return &typeDesc;
    }

    struct TypeDescriptor_Vec3 : TypeDescriptor {
        TypeDescriptor_Vec3() {
            name = "vec3";
            size = sizeof( glm::vec3 );
        }

        virtual nlohmann::json JSON_Write( const void * obj ) override {
            return atto::JSON_Write( *(glm::vec3 *)obj );
        }

        virtual void JSON_Read( const nlohmann::json & j, const void * obj ) override {
            atto::JSON_Read( j, *(glm::vec3 *)obj );
        }

        virtual void Imgui_Draw( const void * obj, const char * memberName ) override {
            ImGui::InputFloat3( memberName, (float *)obj );
        }
    };

    template <>
    TypeDescriptor * GetPrimitiveDescriptor<glm::vec3>() {
        static TypeDescriptor_Vec3 typeDesc;
        return &typeDesc;
    }

    struct TypeDescriptor_Vec4 : TypeDescriptor {
        TypeDescriptor_Vec4() {
            name = "vec4";
            size = sizeof( glm::vec4 );
        }

        virtual nlohmann::json JSON_Write( const void * obj ) override {
            return atto::JSON_Write( *(glm::vec4 *)obj );
        }

        virtual void JSON_Read( const nlohmann::json & j, const void * obj ) override {
            atto::JSON_Read( j, *(glm::vec4 *)obj );
        }

        virtual void Imgui_Draw( const void * obj, const char * memberName ) override {
            ImGui::InputFloat4( memberName, (float *)obj );
        }
    };

    template <>
    TypeDescriptor * GetPrimitiveDescriptor<glm::vec4>() {
        static TypeDescriptor_Vec4 typeDesc;
        return &typeDesc;
    }

    struct TypeDescriptor_Mat2 : TypeDescriptor {
        TypeDescriptor_Mat2() {
            name = "mat2";
            size = sizeof( glm::mat2 );
        }

        virtual nlohmann::json JSON_Write( const void * obj ) override {
            return atto::JSON_Write( *(glm::mat2 *)obj );
        }

        virtual void JSON_Read( const nlohmann::json & j, const void * obj ) override {
            atto::JSON_Read( j, *(glm::mat2 *)obj );
        }

        virtual void Imgui_Draw( const void * obj, const char * memberName ) override {
            throw std::logic_error( "The method or operation is not implemented." );
        }
    };

    template <>
    TypeDescriptor * GetPrimitiveDescriptor<glm::mat2>() {
        static TypeDescriptor_Mat2 typeDesc;
        return &typeDesc;
    }

    struct TypeDescriptor_Mat3 : TypeDescriptor {
        TypeDescriptor_Mat3() {
            name = "mat3";
            size = sizeof( glm::mat3 );
        }

        virtual nlohmann::json JSON_Write( const void * obj ) override {
            return atto::JSON_Write( *(glm::mat3 *)obj );
        }

        virtual void JSON_Read( const nlohmann::json & j, const void * obj ) override {
            atto::JSON_Read( j, *(glm::mat3 *)obj );
        }

        virtual void Imgui_Draw( const void * obj, const char * memberName ) override {
            throw std::logic_error( "The method or operation is not implemented." );
        }
    };

    template <>
    TypeDescriptor * GetPrimitiveDescriptor<glm::mat3>() {
        static TypeDescriptor_Mat3 typeDesc;
        return &typeDesc;
    }

    struct TypeDescriptor_Mat4 : TypeDescriptor {
        TypeDescriptor_Mat4() {
            name = "mat4";
            size = sizeof( glm::mat4 );
        }

        virtual nlohmann::json JSON_Write( const void * obj ) override {
            return atto::JSON_Write( *(glm::mat4 *)obj );
        }

        virtual void JSON_Read( const nlohmann::json & j, const void * obj ) override {
            atto::JSON_Read( j, *(glm::mat4 *)obj );
        }

        virtual void Imgui_Draw( const void * obj, const char * memberName ) override {
            throw std::logic_error( "The method or operation is not implemented." );
        }
    };

    template <>
    TypeDescriptor * GetPrimitiveDescriptor<glm::mat4>() {
        static TypeDescriptor_Mat4 typeDesc;
        return &typeDesc;
    }

    struct TypeDescriptor_SmallString : TypeDescriptor {
        TypeDescriptor_SmallString() {
            name = "smallstring";
            size = sizeof( SmallString );
        }

        virtual nlohmann::json JSON_Write( const void * obj ) override {
            return atto::JSON_Write( *(SmallString *)obj );
        }

        virtual void JSON_Read( const nlohmann::json & j, const void * obj ) override {
            atto::JSON_Read( j, *(SmallString *)obj );
        }

        virtual void Imgui_Draw( const void * obj, const char * memberName ) override {
            SmallString * s = (SmallString *)obj;
            if( ImGui::InputText( memberName, s->GetCStr(), SmallString::CAPCITY ) ) {
                s->CalculateLength();
            }
        }
    };

    template <>
    TypeDescriptor * GetPrimitiveDescriptor<SmallString>() {
        static TypeDescriptor_SmallString typeDesc;
        return &typeDesc;
    }

    struct TypeDescriptor_LargeString : TypeDescriptor {
        TypeDescriptor_LargeString() {
            name = "largestring";
            size = sizeof( LargeString );
        }

        virtual nlohmann::json JSON_Write( const void * obj ) override {
            return atto::JSON_Write( *(LargeString *)obj );
        }

        virtual void JSON_Read( const nlohmann::json & j, const void * obj ) override {
            atto::JSON_Read( j, *(LargeString *)obj );
        }

        virtual void Imgui_Draw( const void * obj, const char * memberName ) override {
            LargeString * s = (LargeString *)obj;
            if( ImGui::InputText( memberName, s->GetCStr(), LargeString::CAPCITY ) ) {
                s->CalculateLength();
            }
        }
    };

    template <>
    TypeDescriptor * GetPrimitiveDescriptor<LargeString>() {
        static TypeDescriptor_LargeString typeDesc;
        return &typeDesc;
    }
}

/*
=====================================================================
===========================MATH OBJECTS==============================
=====================================================================
*/

#include "atto_math.h"

namespace atto {

    struct TypeDescriptor_Collider : TypeDescriptor {
        TypeDescriptor_Collider() {
            name = "Collider";
            size = sizeof( Collider );
        }

        virtual nlohmann::json JSON_Write( const void * obj ) override {
            Collider * collider = (Collider *)obj;
            nlohmann::json j;
            j[ "type" ] = (i32)collider->type;
            switch( collider->type ) {
                case COLLIDER_TYPE_NONE:
                {

                } break;
                case COLLIDER_TYPE_SPHERE:
                {
                    j[ "c" ] = atto::JSON_Write( collider->sphere.c );
                    j[ "r" ] = collider->sphere.r;
                } break;
                case COLLIDER_TYPE_BOX:
                {
                    j[ "min" ] = atto::JSON_Write( collider->box.min );
                    j[ "max" ] = atto::JSON_Write( collider->box.max );
                } break;
                case COLLIDER_TYPE_PLANE:
                {
                    j[ "c" ] = atto::JSON_Write( collider->plane.c );
                    j[ "n" ] = atto::JSON_Write( collider->plane.n );
                } break;
                case COLLIDER_TYPE_TRIANGLE:
                {
                    j[ "p1" ] = atto::JSON_Write( collider->tri.p1 );
                    j[ "p2" ] = atto::JSON_Write( collider->tri.p2 );
                    j[ "p3" ] = atto::JSON_Write( collider->tri.p3 );
                    j[ "n" ] = atto::JSON_Write( collider->tri.n );
                } break;
                default:
                {
                    INVALID_CODE_PATH;
                } break;
            }

            return j;
        }

        virtual void JSON_Read( const nlohmann::json & j, const void * obj ) override {
            Collider * collider = (Collider *)obj;
            collider->type = j[ "type" ];
            switch( collider->type ) {
                case COLLIDER_TYPE_NONE:
                {

                } break;
                case COLLIDER_TYPE_SPHERE:
                {
                    atto::JSON_Read( j[ "c" ], collider->sphere.c );
                    atto::JSON_Read( j[ "r" ], collider->sphere.r );
                } break;
                case COLLIDER_TYPE_BOX:
                {
                    atto::JSON_Read( j[ "min" ], collider->box.min );
                    atto::JSON_Read( j[ "max" ], collider->box.max );
                } break;
                case COLLIDER_TYPE_PLANE:
                {
                    atto::JSON_Read( j[ "c" ], collider->plane.c );
                    atto::JSON_Read( j[ "n" ], collider->plane.n );
                } break;
                case COLLIDER_TYPE_TRIANGLE:
                {
                    atto::JSON_Read( j[ "p1" ], collider->tri.p1 );
                    atto::JSON_Read( j[ "p2" ], collider->tri.p2 );
                    atto::JSON_Read( j[ "p3" ], collider->tri.p3 );
                    atto::JSON_Read( j[ "n" ], collider->tri.n );
                } break;
                default:
                {
                    INVALID_CODE_PATH;
                } break;
            }
        }

        virtual void Imgui_Draw( const void * obj, const char * memberName ) override {
            throw std::logic_error( "The method or operation is not implemented." );
        }

        virtual void Binary_Read( void * obj, BinaryBlob & f ) override {
            throw std::logic_error( "The method or operation is not implemented." );
        }

        virtual void Binary_Write( const void * obj, BinaryBlob & f ) override {
            throw std::logic_error( "The method or operation is not implemented." );
        }
    };

    template <>
    TypeDescriptor * GetPrimitiveDescriptor<Collider>() {
        static TypeDescriptor_Collider typeDesc;
        return &typeDesc;
    }


    struct TypeDescriptor_Collider2D : TypeDescriptor {
        TypeDescriptor_Collider2D() {
            name = "Collider2D";
            size = sizeof( Collider );
        }

        nlohmann::json JSON_Write( const void * obj ) override {
            Collider2D * collider = (Collider2D *)obj;
            nlohmann::json j;
            j[ "type" ] = (i32)collider->type;
            switch( collider->type ) {
                case COLLIDER_TYPE_NONE:
                {

                } break;
                case COLLIDER_TYPE_CIRCLE:
                {
                    j[ "c" ] = atto::JSON_Write( collider->circle.pos );
                    j[ "r" ] = collider->circle.rad;
                } break;
                case COLLIDER_TYPE_SPHERE:
                {
                    INVALID_CODE_PATH;
                } break;
                case COLLIDER_TYPE_BOX:
                {
                    j[ "min" ] = atto::JSON_Write( collider->box.min );
                    j[ "max" ] = atto::JSON_Write( collider->box.max );
                } break;
                case COLLIDER_TYPE_PLANE:
                {
                    INVALID_CODE_PATH;
                } break;
                case COLLIDER_TYPE_TRIANGLE:
                {
                    INVALID_CODE_PATH;
                } break;
                default:
                {
                    INVALID_CODE_PATH;
                } break;
            }

            return j;
        }

        void JSON_Read( const nlohmann::json & j, const void * obj ) override {
            Collider2D * collider = (Collider2D *)obj;
            collider->type = j[ "type" ];
            switch( collider->type ) {
                case COLLIDER_TYPE_NONE:
                {

                } break;
                case COLLIDER_TYPE_CIRCLE:
                {
                    atto::JSON_Read( j[ "c" ], collider->circle.pos );
                    atto::JSON_Read( j[ "r" ], collider->circle.rad );
                } break;
                case COLLIDER_TYPE_SPHERE:
                {
                    INVALID_CODE_PATH;
                } break;
                case COLLIDER_TYPE_BOX:
                {
                    atto::JSON_Read( j[ "min" ], collider->box.min );
                    atto::JSON_Read( j[ "max" ], collider->box.max );
                } break;
                case COLLIDER_TYPE_PLANE:
                {
                    INVALID_CODE_PATH;
                } break;
                case COLLIDER_TYPE_TRIANGLE:
                {
                    INVALID_CODE_PATH;
                } break;
                default:
                {
                    INVALID_CODE_PATH;
                } break;
            }
        }

        void Imgui_Draw( const void * obj, const char * memberName ) override {
            Collider2D * collider = (Collider2D *)obj;
            if( ImGui::TreeNode( memberName ) ) {
                switch( collider->type ) {
                    case COLLIDER_TYPE_NONE:
                    {

                    } break;
                    case COLLIDER_TYPE_CIRCLE:
                    {
                        ImGui::InputFloat2( "Position", &collider->circle.pos.x );
                        ImGui::InputFloat( "Radius", &collider->circle.rad );
                    } break;
                    case COLLIDER_TYPE_SPHERE:
                    {
                        INVALID_CODE_PATH;
                    } break;
                    case COLLIDER_TYPE_BOX:
                    {
                        ImGui::InputFloat2( "Min", &collider->box.min.x );
                        ImGui::InputFloat2( "Max", &collider->box.max.x );
                    } break;
                    case COLLIDER_TYPE_PLANE:
                    {
                        INVALID_CODE_PATH;
                    } break;
                    case COLLIDER_TYPE_TRIANGLE:
                    {
                        INVALID_CODE_PATH;
                    } break;
                    default:
                    {
                        INVALID_CODE_PATH;
                    } break;
                }
                ImGui::TreePop();
            }
        }
    };

    template<>
    TypeDescriptor * GetPrimitiveDescriptor<Collider2D>() {
        static TypeDescriptor_Collider2D typeDesc;
        return &typeDesc;
    }
}

/*
=====================================================================
===========================GAME OBJECTS==============================
=====================================================================
*/

#include "atto_core.h"
#include "../game/modes/atto_game_mode_game.h"

namespace atto {
    /*
    * ====================== TEXTURE PTR
    */
    struct TypeDescriptor_TexturePtr : TypeDescriptor {
        TypeDescriptor_TexturePtr() {
            name = "TexturePtr";
            size = sizeof( TextureResource * );
        }

        virtual nlohmann::json JSON_Write( const void * obj ) override {
            TextureResource * textureResource = *(TextureResource **)obj;
            return atto::JSON_Write( textureResource->name );
        }

        virtual void JSON_Read( const nlohmann::json & j, const void * obj ) override {
        #if ATTO_EDITOR
            LargeString resPath = {};
            atto::JSON_Read( j, resPath );
            TextureResource ** resource = (TextureResource **)obj;
            Core * core = Core::EditorOnly_GetCore();
            *resource = core->ResourceGetAndLoadTexture( resPath.GetCStr() );
        #endif
        }

        virtual void Binary_Write( const void * obj, BinaryBlob & f ) override {
            TextureResource * resource = *(TextureResource **)obj;
            Assert( resource != nullptr );
            f.Write( &resource->name );
        }

        virtual void Binary_Read( void * obj, BinaryBlob & f ) override {
            LargeString resPath = {};
            f.Read( &resPath );
            TextureResource ** resource = (TextureResource **)obj;
            Core * core = Core::EditorOnly_GetCore();
            *resource = core->ResourceGetAndLoadTexture( resPath.GetCStr() );
        }


        virtual void Imgui_Draw( const void * obj, const char * memberName ) override {
        #if ATTO_EDITOR
            TextureResource * textureResource = *(TextureResource **)obj;
            if( textureResource != nullptr ) {
                ImGui::SeparatorText( "Image" );
                if( ImGui::ImageButton( (void *)(intptr_t)textureResource->handle, ImVec2( (f32)textureResource->width, (f32)textureResource->height ) ) ) {
                    Core * core = Core::EditorOnly_GetCore();
                    LargeString resPath = {};
                    if( core->WindowOpenNativeFileDialog( nullptr, "png", resPath ) == true ) {
                        TextureResource ** tptr = (TextureResource **)obj;
                        *tptr = core->ResourceGetAndCreateTexture( resPath.GetCStr(), false, false );
                    }
                }
            }
            else {
                if( ImGui::Button( "No Image" ) ) {
                    Core * core = Core::EditorOnly_GetCore();
                    LargeString resPath = {};
                    if( core->WindowOpenNativeFileDialog( nullptr, "png", resPath ) == true ) {
                        TextureResource ** tptr = (TextureResource **)obj;
                        *tptr = core->ResourceGetAndCreateTexture( resPath.GetCStr(), false, false );
                    }
                }
            }
        #endif
        }
    };

    template <>
    TypeDescriptor * GetPrimitiveDescriptor<TextureResource *>() {
        static TypeDescriptor_TexturePtr typeDesc;
        return &typeDesc;
    }

    /*
    * ====================== AUDIO PTR
    */
    struct TypeDescriptor_AudioPtr : TypeDescriptor {
        TypeDescriptor_AudioPtr() {
            name = "AudioPtr";
            size = sizeof( AudioResource * );
        }

        virtual nlohmann::json JSON_Write( const void * obj ) override {
            AudioResource * audioResource = *(AudioResource **)obj;
            return atto::JSON_Write( audioResource->name );
        }

        virtual void JSON_Read( const nlohmann::json & j, const void * obj ) override {
        #if ATTO_EDITOR
            LargeString resPath = {};
            atto::JSON_Read( j, resPath );
            AudioResource ** resource = (AudioResource **)obj;
            Core * core = Core::EditorOnly_GetCore();
            *resource = core->ResourceGetAndLoadAudio( resPath.GetCStr() );
        #endif
        }

        virtual void Binary_Write( const void * obj, BinaryBlob & f ) override {
            AudioResource * resource = *(AudioResource **)obj;
            Assert( resource != nullptr );
            f.Write( &resource->name );
        }

        virtual void Binary_Read( void * obj, BinaryBlob & f ) override {
            LargeString resPath = {};
            f.Read( &resPath );
            AudioResource ** resource = (AudioResource **)obj;
            Core * core = Core::EditorOnly_GetCore();
            *resource = core->ResourceGetAndLoadAudio( resPath.GetCStr() );
        }

        virtual void Imgui_Draw( const void * obj, const char * memberName ) override {
        #if ATTO_EDITOR
            Core * core = Core::EditorOnly_GetCore();
            AudioResource * audioResource = *(AudioResource **)obj;

            LargeString shortName = audioResource != nullptr ? audioResource->GetShortName() : LargeString::FromLiteral( "Pick" );
            if( ImGui::Button( shortName.GetCStr() ) ) {
                LargeString resPath = {};
                if( core->WindowOpenNativeFileDialog( nullptr, "wav,ogg", resPath ) == true ) {

                }
            }

            if( audioResource != nullptr ) {
                ImGui::SameLine();
                ImGui::PushID( audioResource->id );
                if( ImGui::Button( "Play" ) ) {
                    core->AudioPlay( audioResource );
                }
                ImGui::PopID();
            }
        #endif
        }
    };

    template <>
    TypeDescriptor * GetPrimitiveDescriptor<AudioResource *>() {
        static TypeDescriptor_AudioPtr typeDesc;
        return &typeDesc;
    }

    /*
    * ====================== SPRITE PTR
    */
    struct TypeDescriptor_SpritePtr : TypeDescriptor {
        TypeDescriptor_SpritePtr() {
            name = "SpritePtr";
            size = sizeof( SpriteResource * );
        }

        virtual nlohmann::json JSON_Write( const void * obj ) override {
            SpriteResource * spriteResource = *(SpriteResource **)obj;
            return atto::JSON_Write( spriteResource->spriteName );
        }

        virtual void JSON_Read( const nlohmann::json & j, const void * obj ) override {
        #if ATTO_EDITOR
            LargeString resPath = {};
            atto::JSON_Read( j, resPath );
            SpriteResource ** spriteResource = (SpriteResource **)obj;
            Core * core = Core::EditorOnly_GetCore();
            *spriteResource = core->ResourceGetAndLoadSprite( resPath.GetCStr() );
        #endif
        }

        virtual void Binary_Read( void * obj, BinaryBlob & f ) override {

        }

        virtual void Binary_Write( const void * obj, BinaryBlob & f ) override {
          
        }

        virtual void Imgui_Draw( const void * obj, const char * memberName ) override {
            SpriteResource * spriteResource = *(SpriteResource **)obj;
            if( spriteResource != nullptr || spriteResource->textureResource != nullptr ) {
                ImGui::SeparatorText( "Image" );
                if( ImGui::ImageButton( (void *)(intptr_t)spriteResource->textureResource->handle, ImVec2( (f32)spriteResource->textureResource->width, (f32)spriteResource->textureResource->height ) ) ) {
                    Core * core = Core::EditorOnly_GetCore();
                    LargeString resPath = {};
                    if( core->WindowOpenNativeFileDialog( nullptr, "png", resPath ) == true ) {
                        spriteResource->textureResource = core->ResourceGetAndCreateTexture( resPath.GetCStr(), false, false );
                    }
                }
            }
            else {
                if( ImGui::Button( "No Image" ) ) {
                    Core * core = Core::EditorOnly_GetCore();
                    LargeString resPath = {};
                    if( core->WindowOpenNativeFileDialog( nullptr, "png", resPath ) == true ) {
                        TextureResource ** tptr = (TextureResource **)obj;
                        *tptr = core->ResourceGetAndCreateTexture( resPath.GetCStr(), false, false );
                    }
                }
            }
        }
    };

    template <>
    TypeDescriptor * GetPrimitiveDescriptor<SpriteResource *>() {
        static TypeDescriptor_SpritePtr typeDesc;
        return &typeDesc;
    }
}

