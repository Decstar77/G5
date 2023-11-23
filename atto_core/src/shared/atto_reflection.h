#pragma once

#include "atto_containers.h"
#include "atto_math.h"

#ifndef ATTO_SERVER

#include "json/json.hpp"

namespace atto {

#define ATTO_REFLECT_STRUCT(name)  nlohmann::json JSON_Write(const struct name &obj); \
                            template<> inline name JSON_Read(const nlohmann::json& j);

#define ATTO_REFLECT_ENUM(name) const char * EnumToString(enum class name obj);

#define AF_IGNORE

    nlohmann::json JSON_Write( bool v );
    nlohmann::json JSON_Write( i32 v );
    nlohmann::json JSON_Write( i64 v );
    nlohmann::json JSON_Write( u8 v );
    nlohmann::json JSON_Write( u32 v );
    nlohmann::json JSON_Write( u64 v );
    nlohmann::json JSON_Write( f32 v );
    nlohmann::json JSON_Write( f64 v );

    nlohmann::json JSON_Write( const SmallString & v );
    nlohmann::json JSON_Write( const LargeString & v );

    nlohmann::json JSON_Write( glm::vec2 v );
    nlohmann::json JSON_Write( glm::vec3 v );
    nlohmann::json JSON_Write( glm::vec4 v );
    nlohmann::json JSON_Write( glm::mat2 v );
    nlohmann::json JSON_Write( glm::mat3 v );
    nlohmann::json JSON_Write( glm::mat4 v );

    template<typename _type_, i32 c>
    nlohmann::json JSON_Write( const FixedList<_type_, c> & list ) {
        nlohmann::json j = nlohmann::json::array();

        const int count = list.GetCount();
        for( int i = 0; i < count; ++i ) {
            j.push_back( JSON_Write( list[ i ] ) );
        }

        return j;
    }

    inline void JSON_Read( const nlohmann::json & j, u8 & o ) {
        o = j.get<u8>();
    }

    inline void JSON_Read( const nlohmann::json & j, bool & o ) {
        o = j.get<bool>();
    }

    inline void JSON_Read( const nlohmann::json & j, i32 & o ) {
        o = j.get<i32>();
    }

    inline void JSON_Read( const nlohmann::json & j, i64 & o ) {
        o = j.get<i64>();
    }

    inline void JSON_Read( const nlohmann::json & j, u32 & o ) {
        o = j.get<u32>();
    }

    inline void JSON_Read( const nlohmann::json & j, u64 & o ) {
        o = j.get<u64>();
    }

    inline f32 JSON_Read<f32>( const nlohmann::json & j ) {
        return j.get<f32>();
    }

    inline f64 JSON_Read<f64>( const nlohmann::json & j ) {
        return j.get<f64>();
    }

    inline SmallString JSON_Read<SmallString>( const nlohmann::json & j ) {
        return SmallString::FromLiteral( j.get<std::string>().c_str() );
    }

    inline LargeString JSON_Read<LargeString>( const nlohmann::json & j ) {
        return LargeString::FromLiteral( j.get<std::string>().c_str() );
    }

    inline glm::vec2 JSON_Read<glm::vec2>( const nlohmann::json & j ) {
        return glm::vec2( j.at( "x" ).get<float>(), j.at( "y" ).get<float>() );
    }

    inline glm::vec3 JSON_Read<glm::vec3>( const nlohmann::json & j ) {
        return glm::vec3( j.at( "x" ).get<float>(), j.at( "y" ).get<float>(), j.at( "z" ).get<float>() );
    }

    inline glm::vec4 JSON_Read<glm::vec4>( const nlohmann::json & j ) {
        return glm::vec4( j.at( "x" ).get<float>(), j.at( "y" ).get<float>(), j.at( "z" ).get<float>(), j.at( "w" ).get<float>() );
    }

    inline glm::mat2 JSON_Read<glm::mat2>( const nlohmann::json & j ) {
        return glm::mat2( JSON_Read<glm::vec2>( j.at( "col1" ) ), JSON_Read<glm::vec2>( j.at( "col2" ) ) );
    }

    inline glm::mat3 JSON_Read<glm::mat3>( const nlohmann::json & j ) {
        return glm::mat3( JSON_Read<glm::vec3>( j.at( "col1" ) ), JSON_Read<glm::vec3>( j.at( "col2" ) ), JSON_Read<glm::vec3>( j.at( "col3" ) ) );
    }

    inline glm::mat4 JSON_Read<glm::mat4>( const nlohmann::json & j ) {
        return glm::mat4( JSON_Read<glm::vec4>( j.at( "col1" ) ), JSON_Read<glm::vec4>( j.at( "col2" ) ), JSON_Read<glm::vec4>( j.at( "col3" ) ), JSON_Read<glm::vec4>( j.at( "col4" ) ) );
    }

    template<typename _type_, i32 c>
    inline FixedList<_type_, c> JSON_Read( const  nlohmann::json & j ) {
        FixedList<_type_, c> list = {};

        for( const auto & element : j ) {
            list.Add( JSON_Read<_type_>( element ) );
        }

        return list;
    }

    template<typename _type_>
    inline bool JSON_WriteToFile( const char * path, const _type_ & obj ) {
        nlohmann::json j = JSON_Write( obj );
        return WriteTextFile( path, j.dump().c_str() );
    }

    constexpr i32 REFL_MAX_VAR_COUNT = 100;

    class ReflBase {
    public:
        virtual nlohmann::json  JSON_Save( const void * obj ) const = 0;
        virtual void            JSON_Read( void * obj, const nlohmann::json & j ) const = 0;

    public:
        SmallString name;
        i32 offset;
    };

    template<typename _type_>
    class Refl : public ReflBase {
    public:
        Refl( const char * name, i32 offset, FixedList< ReflBase *, REFL_MAX_VAR_COUNT > & data ) {
            this->name = SmallString::FromLiteral( name );
            this->offset = offset;
            data.Add( this );
        }

        virtual nlohmann::json JSON_Save( const void * obj ) const override {
            const _type_ * t = (const _type_ *)( ( (const byte *)obj ) + offset );
            return JSON_Write( *t );
        }

        virtual void JSON_Read( void * obj, const nlohmann::json & j ) const override {
            _type_ * t = (_type_ *)( ( (byte *)obj ) + offset );
            *t = atto::JSON_Read<_type_>( j );
        }
    };

#define REFL_GET_LIST( name )   _refl_##name##_data
#define REFL_DECLARE( name )    inline FixedList< ReflBase *, REFL_MAX_VAR_COUNT > REFL_GET_LIST( name )
#define REFL_VAR( clss, varr )  inline Refl _refl_##clss##_var_##varr## = Refl< decltype( ##clss##::varr ) >( #varr, (i32)offsetof( clss, varr ), REFL_GET_LIST( clss ) );
#define REFL_WRITE_JSON( path, className, inst ) RelfWriteJsonInternal( path, REFL_GET_LIST( className ), inst )
#define REFL_READ_JSON( path, className, inst) RelfReadJsonInternal( path, REFL_GET_LIST( className ), inst )

    template<typename _type_>
    inline void RelfWriteJsonInternal( const char * path, const FixedList< ReflBase *, REFL_MAX_VAR_COUNT > & varList, const _type_ & inst ) {
        nlohmann::json j;

        const i32 varCount = varList.GetCount();
        for( i32 varIndex = 0; varIndex < varCount; varIndex++ ) {
            ReflBase * r = varList[ varIndex ];
            j[ r->name.GetCStr() ] = r->JSON_Save( &inst );
        }

        WindowsCore::DEBUG_WriteTextFile( path, j.dump().c_str());
    }

    template<typename _type_>
    inline void RelfReadJsonInternal( const char * path, const FixedList< ReflBase *, REFL_MAX_VAR_COUNT > & varList, _type_ & inst ) {
        const i32 mbSize = 16;
        const i32 size = Megabytes( mbSize );
        char * buf = new char[ size ];

        WindowsCore::DEBUG_ReadTextFile( path, buf, size );

        nlohmann::json j = nlohmann::json::parse( buf );
        const i32 varCount = varList.GetCount();
        for( i32 varIndex = 0; varIndex < varCount; varIndex++ ) {
            ReflBase * r = varList[ varIndex ];
            r->JSON_Read( (void *)( &inst ), j[ r->name.GetCStr() ] );
        }

        delete[] buf;
    }
}
#else 

#define ATTO_REFLECT_STRUCT(name)
#define ATTO_REFLECT_ENUM(name)

#endif

