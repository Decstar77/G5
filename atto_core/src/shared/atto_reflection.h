#pragma once

#include "atto_containers.h"
#include "atto_math.h"

#ifndef ATTO_SERVER

#include "json/json.hpp"

namespace atto {

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

    void JSON_Read( const nlohmann::json & j, u8 & o );
    void JSON_Read( const nlohmann::json & j, bool & o );
    void JSON_Read( const nlohmann::json & j, i32 & o );
    void JSON_Read( const nlohmann::json & j, i64 & o );
    void JSON_Read( const nlohmann::json & j, u32 & o );
    void JSON_Read( const nlohmann::json & j, u64 & o );
    void JSON_Read( const nlohmann::json & j, f32 & o );
    void JSON_Read( const nlohmann::json & j, f64 & o );
    void JSON_Read( const nlohmann::json & j, SmallString & o );
    void JSON_Read( const nlohmann::json & j, LargeString & o );
    void JSON_Read( const nlohmann::json & j, glm::vec2 & o );
    void JSON_Read( const nlohmann::json & j, glm::vec3 & o );
    void JSON_Read( const nlohmann::json & j, glm::vec4 & o );
    void JSON_Read( const nlohmann::json & j, glm::mat2 & o );
    void JSON_Read( const nlohmann::json & j, glm::mat3 & o );
    void JSON_Read( const nlohmann::json & j, glm::mat4 & o );

    template<typename _type_, i32 c>
    inline void JSON_Read( const  nlohmann::json & j, FixedList<_type_, c> & list ) {
        for( const auto & element : j ) {
            _type_& t = list.AddEmpty();
            JSON_Read( element, t );
        }
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
            atto::JSON_Read( j, *t );
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

