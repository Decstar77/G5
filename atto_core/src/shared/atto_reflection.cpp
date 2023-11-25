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
}

