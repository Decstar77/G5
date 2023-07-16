#pragma once
#include "../shared/atto_reflection.h"
#include "../shared/atto_core.h"
namespace atto {
    template<>
    GameSettings JSON_Read<GameSettings>(const nlohmann::json &j) {
        GameSettings obj = {};
        obj.windowWidth = JSON_Read<i32>(j["windowWidth"]);
        obj.windowHeight = JSON_Read<i32>(j["windowHeight"]);
        obj.windowStartPosX = JSON_Read<i32>(j["windowStartPosX"]);
        obj.windowStartPosY = JSON_Read<i32>(j["windowStartPosY"]);
        obj.noAudio = JSON_Read<bool>(j["noAudio"]);
        obj.fullscreen = JSON_Read<bool>(j["fullscreen"]);
        obj.vsync = JSON_Read<bool>(j["vsync"]);
        obj.showDebug = JSON_Read<bool>(j["showDebug"]);
        obj.basePath = JSON_Read<SmallString>(j["basePath"]);
        return obj;
    }
}
