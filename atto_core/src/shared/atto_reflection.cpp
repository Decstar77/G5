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

        nlohmann::json JSON_Write( const void * obj ) override {
            return nlohmann::json( *(i32 *)obj );
        }

        void JSON_Read( const nlohmann::json & j, const void * obj ) override {
            *(i32 *)obj = j.get<i32>();
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

        nlohmann::json JSON_Write( const void * obj ) override {
            return nlohmann::json( *(u32 *)obj );
        }

        void JSON_Read( const nlohmann::json & j, const void * obj ) override {
            *(u32 *)obj = j.get<u32>();
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

        nlohmann::json JSON_Write( const void * obj ) override {
            return nlohmann::json( *(i64 *)obj );
        }

        void JSON_Read( const nlohmann::json & j, const void * obj ) override {
            *(i64 *)obj = j.get<i64>();
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

        nlohmann::json JSON_Write( const void * obj ) override {
            return nlohmann::json( *(u64 *)obj );
        }

        void JSON_Read( const nlohmann::json & j, const void * obj ) override {
            *(u64 *)obj = j.get<u64>();
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

        nlohmann::json JSON_Write( const void * obj ) override {
            return nlohmann::json( *(f32 *)obj );
        }

        void JSON_Read( const nlohmann::json & j, const void * obj ) override {
            *(f32 *)obj = j.get<f32>();
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

        nlohmann::json JSON_Write( const void * obj ) override {
            return nlohmann::json( *(f64 *)obj );
        }

        void JSON_Read( const nlohmann::json & j, const void * obj ) override {
            *(f64 *)obj = j.get<f64>();
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

        nlohmann::json JSON_Write( const void * obj ) override {
            return nlohmann::json( *(bool *)obj );
        }

        void JSON_Read( const nlohmann::json & j, const void * obj ) override {
            *(bool *)obj = j.get<bool>();
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

        nlohmann::json JSON_Write( const void * obj ) override {
            return atto::JSON_Write( *(glm::vec2 *)obj );
        }

        void JSON_Read( const nlohmann::json & j, const void * obj ) override {
            atto::JSON_Read( j, *(glm::vec2 *)obj );
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

        nlohmann::json JSON_Write( const void * obj ) override {
            return atto::JSON_Write( *(glm::vec3 *)obj );
        }

        void JSON_Read( const nlohmann::json & j, const void * obj ) override {
            atto::JSON_Read( j, *(glm::vec3 *)obj );
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

        nlohmann::json JSON_Write( const void * obj ) override {
            return atto::JSON_Write( *(glm::vec4 *)obj );
        }

        void JSON_Read( const nlohmann::json & j, const void * obj ) override {
            atto::JSON_Read( j, *(glm::vec4 *)obj );
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

        nlohmann::json JSON_Write( const void * obj ) override {
            return atto::JSON_Write( *(glm::mat2 *)obj );
        }

        void JSON_Read( const nlohmann::json & j, const void * obj ) override {
            atto::JSON_Read( j, *(glm::mat2 *)obj );
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

        nlohmann::json JSON_Write( const void * obj ) override {
            return atto::JSON_Write( *(glm::mat3 *)obj );
        }

        void JSON_Read( const nlohmann::json & j, const void * obj ) override {
            atto::JSON_Read( j, *(glm::mat3 *)obj );
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

        nlohmann::json JSON_Write( const void * obj ) override {
            return atto::JSON_Write( *(glm::mat4 *)obj );
        }

        void JSON_Read( const nlohmann::json & j, const void * obj ) override {
            atto::JSON_Read( j, *(glm::mat4 *)obj );
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

        nlohmann::json JSON_Write( const void * obj ) override {
            return atto::JSON_Write( *(SmallString*)obj );
        }

        void JSON_Read( const nlohmann::json & j, const void * obj ) override {
            atto::JSON_Read( j, *(SmallString *)obj );
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

        nlohmann::json JSON_Write( const void * obj ) override {
            return atto::JSON_Write( *(LargeString *)obj );
        }

        void JSON_Read( const nlohmann::json & j, const void * obj ) override {
            atto::JSON_Read( j, *(LargeString *)obj );
        }
    };

    template <>
    TypeDescriptor * GetPrimitiveDescriptor<LargeString>() {
        static TypeDescriptor_LargeString typeDesc;
        return &typeDesc;
    }
}

