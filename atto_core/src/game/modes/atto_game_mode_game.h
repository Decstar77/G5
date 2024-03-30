#pragma once

#include "../atto_game.h"
#include "../sim/atto_sim_map.h"

namespace atto {
    
    struct GameStartParams {
        SmallString mapName;
        bool        isMutliplayer;
        i32         localPlayerNumber;
        i32         otherPlayerNumber;
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
