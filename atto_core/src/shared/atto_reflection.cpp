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

        virtual LargeString ToString( const void * obj ) override {
            LargeString res = {};
            res.Add( std::to_string( *( (i32 *)obj ) ).c_str() );
            return res;
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

        virtual LargeString ToString( const void * obj ) override {
            LargeString res = {};
            res.Add( std::to_string( *( (u32 *)obj ) ).c_str() );
            return res;
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

        virtual LargeString ToString( const void * obj ) override {
            LargeString res = {};
            res.Add( std::to_string( *( (i64 *)obj ) ).c_str() );
            return res;
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

        virtual LargeString ToString( const void * obj ) override {
            LargeString res = {};
            res.Add( std::to_string( *( (u64 *)obj ) ).c_str() );
            return res;
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

        virtual LargeString ToString( const void * obj ) override {
            LargeString res = {};
            res.Add( std::to_string( *( (f32 *)obj ) ).c_str() );
            return res;
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

        virtual LargeString ToString( const void * obj ) override {
            LargeString res = {};
            res.Add( std::to_string( *( (f64 *)obj ) ).c_str() );
            return res;
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

        virtual LargeString ToString( const void * obj ) override {
            LargeString res = {};
            res.Add( std::to_string( *( (bool *)obj ) ).c_str() );
            return res;
        }
    };

    template <>
    TypeDescriptor * GetPrimitiveDescriptor<bool>() {
        static TypeDescriptor_Bool typeDesc;
        return &typeDesc;
    }

    struct TypeDescriptor_FixedPoint : TypeDescriptor {
        TypeDescriptor_FixedPoint() {
            name = "fp";
            size = sizeof( fp );
        }

        virtual nlohmann::json JSON_Write( const void * obj ) override {
            INVALID_CODE_PATH;
            return nlohmann::json();
        }

        virtual void JSON_Read( const nlohmann::json & j, const void * obj ) override {
            INVALID_CODE_PATH;
        }

        virtual LargeString ToString( const void * obj ) override {
            LargeString res = {};
            fp f = *(fp *)obj;
            f32 v = ToFloat( f );
            res.Add( std::to_string( v ).c_str() );
            return res;
        }
    };

    template <>
    TypeDescriptor * GetPrimitiveDescriptor<fp>() {
        static TypeDescriptor_FixedPoint typeDesc;
        return &typeDesc;
    }

    struct TypeDescriptor_Fp2 : TypeDescriptor {
        TypeDescriptor_Fp2() {
            name = "fp2";
            size = sizeof( fp2 );
        }

        virtual nlohmann::json JSON_Write( const void * obj ) override {
            INVALID_CODE_PATH;
            return nlohmann::json();
        }

        virtual void JSON_Read( const nlohmann::json & j, const void * obj ) override {
            INVALID_CODE_PATH;
        }

        virtual LargeString ToString( const void * obj ) override {
            LargeString res = {};
            fp2 v = *(fp2 *)obj;
            res.Add( "< " );
            res.Add( std::to_string( ToFloat( v.x ) ).c_str() );
            res.Add( ", " );
            res.Add( std::to_string( ToFloat( v.x ) ).c_str() );
            res.Add( " >" );
            return res;
        }
    };

    template <>
    TypeDescriptor * GetPrimitiveDescriptor<fp2>() {
        static TypeDescriptor_Fp2 typeDesc;
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

        virtual LargeString ToString( const void * obj ) override {
            LargeString res = {};
            glm::vec2 v = *(glm::vec2 *)obj;
            res.Add( "< " );
            res.Add( std::to_string( v.x ).c_str() );
            res.Add( ", " );
            res.Add( std::to_string( v.y ).c_str() );
            res.Add( " >" );
            return res;
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

        virtual LargeString ToString( const void * obj ) override {
            LargeString res = {};
            glm::vec3 v = *(glm::vec3 *)obj;
            res.Add( "< " );
            res.Add( std::to_string( v.x ).c_str() );
            res.Add( ", " );
            res.Add( std::to_string( v.y ).c_str() );
            res.Add( ", " );
            res.Add( std::to_string( v.z ).c_str() );
            res.Add( " >" );
            return res;
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

        virtual LargeString ToString( const void * obj ) override {
            LargeString res = {};
            glm::vec4 v = *(glm::vec4 *)obj;
            res.Add( "< " );
            res.Add( std::to_string( v.x ).c_str() );
            res.Add( ", " );
            res.Add( std::to_string( v.y ).c_str() );
            res.Add( ", " );
            res.Add( std::to_string( v.z ).c_str() );
            res.Add( ", " );
            res.Add( std::to_string( v.w ).c_str() );
            res.Add( " >" );
            return res;
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

        virtual LargeString ToString( const void * obj ) override {
            INVALID_CODE_PATH;
            return {};
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

        virtual LargeString ToString( const void * obj ) override {
            INVALID_CODE_PATH;
            return {};
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

        virtual LargeString ToString( const void * obj ) override {
            INVALID_CODE_PATH;
            return {};
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

        virtual LargeString ToString( const void * obj ) override {
            SmallString str = *(SmallString *)obj;
            LargeString res = {};
            res.Add( str.GetCStr() );
            return res;
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

        virtual LargeString ToString( const void * obj ) override {
            LargeString str = *(LargeString *)obj;
            return str;
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
                case COLLIDER_TYPE_AXIS_BOX:
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
                case COLLIDER_TYPE_AXIS_BOX:
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

        virtual void Binary_Read( void * obj, BinaryBlob & f ) override {
            throw std::logic_error( "The method or operation is not implemented." );
        }

        virtual void Binary_Write( const void * obj, BinaryBlob & f ) override {
            throw std::logic_error( "The method or operation is not implemented." );
        }

        virtual LargeString ToString( const void * obj ) override {
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
            size = sizeof( Collider2D );
        }

        nlohmann::json JSON_Write( const void * obj ) override {
            Collider2D * collider = (Collider2D *)obj;
            nlohmann::json j;
            j[ "type" ] = (i32)collider->type;
            switch( collider->type ) {
                case COLLIDER_TYPE_NONE: { } break;
                case COLLIDER_TYPE_CIRCLE:
                {
                    j[ "c" ] = atto::JSON_Write( collider->circle.pos );
                    j[ "r" ] = collider->circle.rad;
                } break;
                case COLLIDER_TYPE_SPHERE: { INVALID_CODE_PATH; } break;
                case COLLIDER_TYPE_AXIS_BOX:
                {
                    j[ "min" ] = atto::JSON_Write( collider->box.min );
                    j[ "max" ] = atto::JSON_Write( collider->box.max );
                } break;
                case COLLIDER_TYPE_PLANE: { INVALID_CODE_PATH; } break;
                case COLLIDER_TYPE_TRIANGLE: { INVALID_CODE_PATH; } break;
                default: { INVALID_CODE_PATH; } break;
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
                case COLLIDER_TYPE_AXIS_BOX:
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

        virtual LargeString ToString( const void * obj ) override {
            throw std::logic_error( "The method or operation is not implemented." );
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
=========================RESOURCE OBJECTS============================
=====================================================================
*/

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
            LargeString resPath = {};
            atto::JSON_Read( j, resPath );
            TextureResource ** textureResource = ( TextureResource * * )obj;
            *textureResource = ResourceGetAndCreateTexture( resPath.GetCStr() );
        }

        virtual LargeString ToString( const void * obj ) override {
            throw std::logic_error( "The method or operation is not implemented." );
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
        }

        virtual LargeString ToString( const void * obj ) override {
            throw std::logic_error( "The method or operation is not implemented." );
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
            return atto::JSON_Write( spriteResource->name );
        }

        virtual void JSON_Read( const nlohmann::json & j, const void * obj ) override {
            INVALID_CODE_PATH
        }

        virtual void Binary_Read( void * obj, BinaryBlob & f ) override {
            INVALID_CODE_PATH
        }

        virtual void Binary_Write( const void * obj, BinaryBlob & f ) override {
            INVALID_CODE_PATH
        }

        virtual LargeString ToString( const void * obj ) override {
            throw std::logic_error( "The method or operation is not implemented." );
        }
    };

    template <>
    TypeDescriptor * GetPrimitiveDescriptor<SpriteResource *>() {
        static TypeDescriptor_SpritePtr typeDesc;
        return &typeDesc;
    }
}

