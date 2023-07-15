
#include "atto_game_state.h"

namespace atto {

    void GameState::Start(Core* core) {
        AudioResource* audio = core->ResourceGetAndLoadAudio("basic_death_1.wav");
        core->AudioPlay(audio, 1.0f);
    }

    void GameState::UpdateAndRender(Core* core) {
        if (core->InputKeyJustPressed(KEY_CODE_ESCAPE)) {
            core->WindowClose();
        }

        Camera camera = core->RenderCreateCamera();

        TextureResource *res = core->ResourceGetAndLoadTexture("unit_basic_man.png");
        static glm::vec2 p = glm::vec2(0);
        p.x += 0.1f;
        //core->RenderDrawRect(glm::vec2(200), glm::vec2(100), 0.0f, glm::vec4(1, 0, 0, 1));

        

        core->RenderSetCamera(&camera);
        core->RenderDrawSprite(res, p, glm::vec2(1));
        core->RenderDrawCircle(p, 5);
        core->RenderSetCamera(nullptr);
        core->RenderSubmit();
    }

    void GameState::Shutdown(Core* core) {

    }

}

