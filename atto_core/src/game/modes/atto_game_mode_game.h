#pragma once

#include "../atto_game.h"
#include "../vis/atto_vis_map.h"
#include "../atto_map_communicator.h"

namespace atto {
    

    struct GameStartParams {
        SmallString mapName;
        bool        isMutliplayer;
        i32         localPlayerNumber;
        i32         otherPlayerNumber;
    };

    struct GameGUI {
        f32             startX;
        f32             startY;
        DrawContext *   drawContext;
        Core *          core;

        void BeginAbilityBar( Core * core, DrawContext * drawContext );
        void EndAbilityBar();
    };

    class GameMode_SinglePlayerGame : public GameMode {
    public:
        GameMode_SinglePlayerGame( GameStartParams parms ) : startParms( parms ) {};
        virtual GameModeType                    GetGameModeType() override;
        virtual bool                            IsInitialized() override;
        virtual void                            Initialize( Core * core ) override;
        virtual void                            UpdateAndRender( Core * core, f32 dt, UpdateAndRenderFlags flags ) override;
        virtual void                            Shutdown( Core * core ) override;

    public:
        GameStartParams                         startParms;
        SimMap                                  simMap;
    };

    class GameMode_MultiplayerGame : public GameMode {
    public:
        GameMode_MultiplayerGame( GameStartParams parms ) : startParms( parms ) {};
        virtual GameModeType                    GetGameModeType() override;
        virtual bool                            IsInitialized() override;
        virtual void                            Initialize( Core * core ) override;
        virtual void                            UpdateAndRender( Core * core, f32 dt, UpdateAndRenderFlags flags ) override;
        virtual void                            Shutdown( Core * core ) override;

    public:
        GameStartParams                         startParms;
        bool                                    isHost;
        SimMap                                  simMap;
    };
}
