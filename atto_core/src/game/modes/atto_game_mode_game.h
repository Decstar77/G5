#pragma once

#include "../atto_game.h"
#include "../../sim/atto_sim_map.h"
#include "../../sim/atto_sim_communicator.h"
#include "../atto_vis_map.h"

namespace atto {
    
    struct GameStartParams {
        SmallString mapName;
        bool        isMutliplayer;
        i32         localPlayerNumber;
        i32         otherPlayerNumber;
    };

    class SimCommunicator_SinglePlayer : public SimCommunicator {
    public:
        virtual void SendToAllClients( MapActionBuffer * actionBuffer ) override;

    public:
        VisMap * visMap = nullptr;
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
        SimCommunicator_SinglePlayer            communicator;
        VisMap                                  visMap;
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
        VisMap                                  visMap;
    };
}
