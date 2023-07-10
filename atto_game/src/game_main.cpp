#include "atto_game_state.h"

using namespace atto;
extern "C" __declspec(dllexport) u64 GameSize() {
    return sizeof(GameState);
}

extern "C" __declspec(dllexport) void GameStart(Core * core) {
    core->UserData = core->MemoryAllocatePermanentCPP<GameState>();
    GameState* state = (GameState*)core->UserData;
    state->Start(core);
}

extern "C" __declspec(dllexport) void GameUpdateAndRender(Core* core) {
    GameState* state = (GameState*)core->UserData;
    state->UpdateAndRender(core);
}

extern "C" __declspec(dllexport) void GameShutdown(Core * core) {
    GameState* state = (GameState*)core->UserData;
    state->Shutdown(core);
}



