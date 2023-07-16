#include "atto_core_windows.h"

#include <string>
#include <Windows.h>

extern "C" {
    __declspec(dllexport) DWORD NvOptimusEnablement = 1;
    __declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}

namespace atto {
    inline static std::string WcharToAnsi(WCHAR const* str, size_t len) {
        int const size_needed = WideCharToMultiByte(CP_UTF8, 0, str, (int)len, NULL, 0, NULL, NULL);
        std::string strTo(size_needed, 0);
        WideCharToMultiByte(CP_UTF8, 0, str, (int)len, &strTo[0], size_needed, NULL, NULL);
        return strTo;
    }

    static HMODULE hDll = NULL;
    bool WindowsCore::OsLoadDLL(GameCodeAPI& gameCode) {
        if (hDll != NULL) {
            FreeLibrary(hDll);
        }

        CopyFileA("x86_64/atto_game.dll", "x86_64/atto_game_temp.dll", FALSE);
        hDll = LoadLibraryA("atto_game_temp.dll");
        if (hDll == NULL) {
            return false;
        }

        gameCode.gameSize = (GameCodeAPI::GameSize)GetProcAddress(hDll, "GameSize");
        gameCode.gameStart = (GameCodeAPI::GameStart)GetProcAddress(hDll, "GameStart");
        gameCode.gameUpdateAndRender = (GameCodeAPI::GameUpdateAndRender)GetProcAddress(hDll, "GameUpdateAndRender");
        gameCode.gameShutdown = (GameCodeAPI::GameShutdown)GetProcAddress(hDll, "GameShutdown");
        gameCode.gameSimStart = (GameCodeAPI::GameSimStart)GetProcAddress(hDll, "GameSimStart");
        gameCode.gameSimStep = (GameCodeAPI::GameSimStep)GetProcAddress(hDll, "GameSimStep");
        gameCode.gameSimSave = (GameCodeAPI::GameSimSave)GetProcAddress(hDll, "GameSimSave");
        gameCode.gameSimLoad = (GameCodeAPI::GameSimLoad)GetProcAddress(hDll, "GameSimLoad");

        return gameCode.gameStart != NULL && gameCode.gameUpdateAndRender != NULL && gameCode.gameShutdown != NULL;
    }

    void WindowsCore::OsLogMessage(const char* message, u8 colour) {
        HANDLE console_handle = GetStdHandle(STD_OUTPUT_HANDLE);
        // @NOTE: FATAL, ERROR, WARN, INFO, DEBUG, TRACE
        static u8 levels[6] = { 64, 4, 6, 2, 1, 8 };
        SetConsoleTextAttribute(console_handle, levels[colour]);
        OutputDebugStringA(message);
        u64 length = strlen(message);
        LPDWORD number_written = 0;
        WriteConsoleA(GetStdHandle(STD_OUTPUT_HANDLE),
            message,
            (DWORD)length,
            number_written,
            0
        );
        SetConsoleTextAttribute(console_handle, 8);
    }

    void WindowsCore::OsErrorBox(const char* msg) {
        MessageBeep(MB_ICONERROR);
        MessageBoxA(NULL, msg, "Catastrophic Error !!! (BOOOM) ", MB_ICONERROR);
    }

    static unsigned long long FileTimeToUInt64(const FILETIME& fileTime) {
        ULARGE_INTEGER dateTime;

        dateTime.LowPart = fileTime.dwLowDateTime;
        dateTime.HighPart = fileTime.dwHighDateTime;

        return dateTime.QuadPart;
    }

    u64 WindowsCore::OsGetFileLastWriteTime(const char* fileName) {
        FILETIME lastWriteTime = {};
        HANDLE fileHandle = CreateFileA(fileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);

        if (fileHandle == INVALID_HANDLE_VALUE) {
            LogOutput(LogLevel::ERR, "OsGetFileLastWriteTime :: Failed to open file %s. Error: %d", fileName, GetLastError());
        }
        else {
            if (GetFileTime(fileHandle, NULL, NULL, &lastWriteTime) == 0) {
                LogOutput(LogLevel::ERR, "OsGetFileLastWriteTime :: Failed to get file time, %s. Error: %d", fileName, GetLastError());
            }
            CloseHandle(fileHandle);
        }

        return FileTimeToUInt64(lastWriteTime);
    }
}
