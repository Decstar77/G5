#pragma once

#include "atto_containers.h"
#include "atto_math.h"
#include "atto_math_fixed.h"

#ifndef ATTO_SERVER

#include "json/json.hpp"

namespace atto {

    template<typename T>
    inline T JSON_Read(const nlohmann::json& j) {
        static_assert(false, "No JSON_Read specialization for this type");
        return T();
    }

#define ATTO_REFLECT_STRUCT(name)  nlohmann::json JSON_Write(const struct name &obj); \
                            template<> inline name JSON_Read(const nlohmann::json& j);

#define ATTO_REFLECT_ENUM(name) const char * EnumToString(enum class name obj);

#define AF_IGNORE

    nlohmann::json JSON_Write(bool v);
    nlohmann::json JSON_Write(i32 v);
    nlohmann::json JSON_Write(i64 v);
    nlohmann::json JSON_Write(u8 v);
    nlohmann::json JSON_Write(u32 v);
    nlohmann::json JSON_Write(u64 v);
    nlohmann::json JSON_Write(f32 v);
    nlohmann::json JSON_Write(f64 v);
    nlohmann::json JSON_Write(fp v);

    nlohmann::json JSON_Write(const SmallString& v);
    nlohmann::json JSON_Write(const LargeString& v);

    nlohmann::json JSON_Write(glm::vec2 v);
    nlohmann::json JSON_Write(glm::vec3 v);
    nlohmann::json JSON_Write(glm::vec4 v);
    nlohmann::json JSON_Write(glm::mat2 v);
    nlohmann::json JSON_Write(glm::mat3 v);
    nlohmann::json JSON_Write(glm::mat4 v);
    nlohmann::json JSON_Write(fpv2 v);

    template<typename _type_, i32 c>
    nlohmann::json JSON_Write(const FixedList<_type_, c>& list) {
        nlohmann::json j = nlohmann::json::array();

        const int count = list.GetCount();
        for (int i = 0; i < count; ++i) {
            j.push_back(JSON_Write(list[i]));
        }

        return j;
    }
    
    template<>
    inline u8 JSON_Read<u8>(const nlohmann::json& j) {
        return j.get<u8>();
    }

    template<>
    inline bool JSON_Read<bool>(const nlohmann::json& j) {
        return j.get<bool>();
    }

    template<>
    inline i32 JSON_Read<i32>(const nlohmann::json& j) {
        return j.get<i32>();
    }

    template<>
    inline i64 JSON_Read<i64>(const nlohmann::json& j) {
        return j.get<i64>();
    }

    template<>
    inline u32 JSON_Read<u32>(const nlohmann::json& j) {
        return j.get<u32>();
    }

    template<>
    inline u64 JSON_Read<u64>(const nlohmann::json& j) {
        return j.get<u64>();
    }

    template<>
    inline f32 JSON_Read<f32>(const nlohmann::json& j) {
        return j.get<f32>();
    }

    template<>
    inline f64 JSON_Read<f64>(const nlohmann::json& j) {
        return j.get<f64>();
    }

    template<>
    inline fp JSON_Read<fp>(const nlohmann::json& j) {
        fp f = {};
        f.value = j.get<i32>();
        return f;
    }

    template<>
    inline SmallString JSON_Read<SmallString>(const nlohmann::json& j) {
        return SmallString::FromLiteral(j.get<std::string>().c_str());
    }

    template<>
    inline LargeString JSON_Read<LargeString>(const nlohmann::json& j) {
        return LargeString::FromLiteral(j.get<std::string>().c_str());
    }

    template<>
    inline glm::vec2 JSON_Read<glm::vec2>(const nlohmann::json& j) {
        return glm::vec2(j.at("x").get<float>(), j.at("y").get<float>());
    }

    template<>
    inline glm::vec3 JSON_Read<glm::vec3>(const nlohmann::json& j) {
        return glm::vec3(j.at("x").get<float>(), j.at("y").get<float>(), j.at("z").get<float>());
    }

    template<>
    inline glm::vec4 JSON_Read<glm::vec4>(const nlohmann::json& j) {
        return glm::vec4(j.at("x").get<float>(), j.at("y").get<float>(), j.at("z").get<float>(), j.at("w").get<float>());
    }

    template<>
    inline glm::mat2 JSON_Read<glm::mat2>(const nlohmann::json& j) {
        return glm::mat2(JSON_Read<glm::vec2>(j.at("col1")), JSON_Read<glm::vec2>(j.at("col2")));
    }

    template<>
    inline glm::mat3 JSON_Read<glm::mat3>(const nlohmann::json& j) {
        return glm::mat3(JSON_Read<glm::vec3>(j.at("col1")), JSON_Read<glm::vec3>(j.at("col2")), JSON_Read<glm::vec3>(j.at("col3")));
    }

    template<>
    inline glm::mat4 JSON_Read<glm::mat4>(const nlohmann::json& j) {
        return glm::mat4(JSON_Read<glm::vec4>(j.at("col1")), JSON_Read<glm::vec4>(j.at("col2")), JSON_Read<glm::vec4>(j.at("col3")), JSON_Read<glm::vec4>(j.at("col4")));
    }

    template<>
    inline fpv2 JSON_Read<fpv2>(const nlohmann::json& j) {
        fpv2 v = {};
        v.x.value = j.at("x").get<i32>();
        v.y.value = j.at("y").get<i32>();
        return v;
    }

    template<typename _type_, i32 c>
    inline FixedList<_type_, c> JSON_Read_FixedList(const  nlohmann::json& j) {
        FixedList<_type_, c> list = {};

        for (const auto& element : j) {
            list.Add( JSON_Read<_type_>( element ) );
        }

        return list;
    }

    template<typename _type_>
    inline bool JSON_WriteToFile(const char* path, const _type_& obj) {
        nlohmann::json j = JSON_Write(obj);
        return WriteTextFile(path, j.dump().c_str());
    }
}
#else 

#define ATTO_REFLECT_STRUCT(name)
#define ATTO_REFLECT_ENUM(name)

#endif

