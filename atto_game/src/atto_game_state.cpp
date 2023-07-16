
#include "atto_game_state.h"

namespace atto {

    void GameState::Start(Core* core) {
        AudioResource* audio = core->ResourceGetAndLoadAudio("basic_death_1.wav");
        core->AudioPlay(audio, 1.0f);

        arialFont = core->ResourceGetAndLoadFont("arial.ttf", 24);
        debugText = "Hello ?";
    }

    void GameState::UpdateAndRender(Core* core) {
        if (core->InputKeyJustPressed(KEY_CODE_F1)) {
            core->NetConnect();
        }

        if (core->InputKeyJustPressed(KEY_CODE_ESCAPE)) {
            core->WindowClose();
        }

        core->RenderDrawText(arialFont, glm::vec2(100, 100), debugText.GetCStr());
        
        core->RenderDrawText(arialFont, glm::vec2(100, 120), 
            StringFormat::Small("PlayerNumber = %d", core->localPlayerNumber).GetCStr());

        core->RenderDrawText(arialFont, glm::vec2(100, 140),
            StringFormat::Small("Ping = %d", core->NetGetPing()).GetCStr());

        Camera camera = core->RenderCreateCamera();

        if (core->localPlayerNumber == 1) {
            if (core->InputKeyDown(KEY_CODE_A)) {
                core->p1Pos.x -= 1;
            }
            if (core->InputKeyDown(KEY_CODE_D)) {
                core->p1Pos.x += 1;
            }
            if (core->InputKeyDown(KEY_CODE_W)) {
                core->p1Pos.y += 1;
            }
            if (core->InputKeyDown(KEY_CODE_S)) {
                core->p1Pos.y -= 1;
            }
        }
        else {
            if (core->InputKeyDown(KEY_CODE_A)) {
                core->p2Pos.x -= 1;
            }
            if (core->InputKeyDown(KEY_CODE_D)) {
                core->p2Pos.x += 1;
            }
            if (core->InputKeyDown(KEY_CODE_W)) {
                core->p2Pos.y += 1;
            }
            if (core->InputKeyDown(KEY_CODE_S)) {
                core->p2Pos.y -= 1;
            }
        }

        TextureResource *res = core->ResourceGetAndLoadTexture("unit_basic_man.png");
        //core->RenderDrawRect(glm::vec2(200), glm::vec2(100), 0.0f, glm::vec4(1, 0, 0, 1));

        core->RenderSetCamera(&camera);
        core->RenderDrawSprite(res, core->p1Pos, glm::vec2(1));
        core->RenderDrawSprite(res, core->p2Pos, glm::vec2(1));
        core->RenderSetCamera(nullptr);
        core->RenderSubmit();
    }

    void GameState::Shutdown(Core* core) {

    }

    void GameState::SimStart(Core* core) {
        core->LogOutput(LogLevel::INFO, "SImSTART");
        debugText = "Con";
    }

    void GameState::SimStep(Core* core, const FixedList<i32, 2>& inputs) {
        
    }

    void GameState::SimSave(Core* core, void** buffer, i64& size, i64& checkSum) {

    }

    void GameState::SimLoad(Core* core, void* buffer, i64 size) {

    }
}

