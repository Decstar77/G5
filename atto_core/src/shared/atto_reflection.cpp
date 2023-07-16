#include "atto_reflection.h"

namespace atto {
    nlohmann::json JSON_Write(bool v) {
        return nlohmann::json(v);
    }

    nlohmann::json JSON_Write(i32 v) {
        return nlohmann::json(v);
    }
    
    nlohmann::json JSON_Write(i64 v) {
        return nlohmann::json(v);
    }

    nlohmann::json JSON_Write(u8 v) {
        return nlohmann::json(v);
    }

    nlohmann::json JSON_Write(u32 v) {
        return nlohmann::json(v);
    }

    nlohmann::json JSON_Write(u64 v) {
        return nlohmann::json(v);
    }

    nlohmann::json JSON_Write(f32 v) {
        return nlohmann::json(v);
    }

    nlohmann::json JSON_Write(f64 v) {
        return nlohmann::json(v);
    }
    
    nlohmann::json JSON_Write(fp v) {
        return nlohmann::json(v.value);
    }

    nlohmann::json JSON_Write(const SmallString& v) {
        return nlohmann::json(v.GetCStr());
    }

    nlohmann::json JSON_Write(const LargeString& v) {
        return nlohmann::json(v.GetCStr());
    }

    nlohmann::json JSON_Write(glm::vec2 v) {
        nlohmann::json j = {};
        j["x"] = v.x;
        j["y"] = v.y;
        return j;
    }

    nlohmann::json JSON_Write(glm::vec3 v) {
        nlohmann::json j = {};
        j["x"] = v.x;
        j["y"] = v.y;
        j["z"] = v.z;
        return j;
    }
    
    nlohmann::json JSON_Write(glm::vec4 v) {
        nlohmann::json j = {};
        j["x"] = v.x;
        j["y"] = v.y;
        j["z"] = v.z;
        j["w"] = v.w;
        return j;
    }

    nlohmann::json JSON_Write(glm::mat2 v) {
        nlohmann::json j = {};
        j["col1"] = JSON_Write(v[0]);
        j["col2"] = JSON_Write(v[1]);
        return j;
    }

    nlohmann::json JSON_Write(glm::mat3 v) {
        nlohmann::json j = {};
        j["col1"] = JSON_Write(v[0]);
        j["col2"] = JSON_Write(v[1]);
        j["col3"] = JSON_Write(v[2]);
        return j;
    }

    nlohmann::json JSON_Write(glm::mat4 v) {
        nlohmann::json j = {};
        j["col1"] = JSON_Write(v[0]);
        j["col2"] = JSON_Write(v[1]);
        j["col3"] = JSON_Write(v[2]);
        j["col4"] = JSON_Write(v[3]);
        return j;
    }

    nlohmann::json JSON_Write(fpv2 v) {
        nlohmann::json j = {};
        j["x"] = v.x.value;
        j["y"] = v.y.value;
        return j;
    }

    u8 JSON_Read_u8(const nlohmann::json& j) {
        return j.get<u8>();
    }

    bool JSON_Read_bool(const nlohmann::json& j) {
        return j.get<bool>();
    }

    i32 JSON_Read_i32(const nlohmann::json& j) {
        return j.get<i32>();
    }

    i64 JSON_Read_i64(const nlohmann::json& j) {
        return j.get<i64>();
    }

    u32 JSON_Read_u32(const nlohmann::json& j) {
        return j.get<u32>();
    }

    u64 JSON_Read_u64(const nlohmann::json& j) {
        return j.get<u64>();
    }

    f32 JSON_Read_f32(const nlohmann::json& j) {
        return j.get<f32>();
    }

    f64 JSON_Read_f64(const nlohmann::json& j) {
        return j.get<f64>();
    }

    SmallString JSON_Read_SmallString(const nlohmann::json& j) {
        return SmallString::FromLiteral(j.get<std::string>().c_str());
    }

    LargeString JSON_Read_LargeString(const nlohmann::json& j) {
        return LargeString::FromLiteral(j.get<std::string>().c_str());
    }

    glm::vec2 JSON_Read_vec2(const nlohmann::json& j) {
        return glm::vec2(j.at("x").get<float>(), j.at("y").get<float>());
    }

    glm::vec3 JSON_Read_vec3(const nlohmann::json& j) {
        return glm::vec3(j.at("x").get<float>(), j.at("y").get<float>(), j.at("z").get<float>());
    }

    glm::vec4 JSON_Read_vec4(const nlohmann::json& j) {
        return glm::vec4(j.at("x").get<float>(), j.at("y").get<float>(), j.at("z").get<float>(), j.at("w").get<float>());
    }

    glm::mat2 JSON_Read_mat2(const nlohmann::json& j) {
        return glm::mat2(JSON_Read_vec2(j.at("col1")), JSON_Read_vec2(j.at("col2")));
    }

    glm::mat3 JSON_Read_mat3(const nlohmann::json& j) {
        return glm::mat3(JSON_Read_vec3(j.at("col1")), JSON_Read_vec3(j.at("col2")), JSON_Read_vec3(j.at("col3")));
    }

    glm::mat4 JSON_Read_mat4(const nlohmann::json& j) {
        return glm::mat4(JSON_Read_vec4(j.at("col1")), JSON_Read_vec4(j.at("col2")), JSON_Read_vec4(j.at("col3")), JSON_Read_vec4(j.at("col4")));
    }
}

