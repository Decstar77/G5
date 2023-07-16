#include "atto_reflection_gen.h"
namespace atto {
    nlohmann::json JSON_Write(const GameSettings& obj) {
        nlohmann::json j = {};
        j["windowWidth"] = JSON_Write(obj.windowWidth);
        j["windowHeight"] = JSON_Write(obj.windowHeight);
        j["windowStartPosX"] = JSON_Write(obj.windowStartPosX);
        j["windowStartPosY"] = JSON_Write(obj.windowStartPosY);
        j["noAudio"] = JSON_Write(obj.noAudio);
        j["fullscreen"] = JSON_Write(obj.fullscreen);
        j["vsync"] = JSON_Write(obj.vsync);
        j["showDebug"] = JSON_Write(obj.showDebug);
        j["basePath"] = JSON_Write(obj.basePath);
        return j;
    }
}
