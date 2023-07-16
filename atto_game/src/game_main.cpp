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

extern "C" __declspec(dllexport) void GameSimStart(Core * core) {
    GameState* state = (GameState*)core->UserData;
    state->SimStart(core);
}

extern "C" __declspec(dllexport) void GameSimStep(Core * core, const FixedList<i32, 2> &inputs) {
    GameState* state = (GameState*)core->UserData;
    state->SimStep(core, inputs);
}

extern "C" __declspec(dllexport) void GameSimSave(Core * core, void** buffer, i64 & size, i64 & checkSum) {
    GameState* state = (GameState*)core->UserData;
    state->SimSave(core, buffer, size, checkSum);
}

extern "C" __declspec(dllexport) void GameSimLoad(Core * core, void* buffer, i64 size) {
    GameState* state = (GameState*)core->UserData;
    state->SimLoad(core, buffer, size);
}


