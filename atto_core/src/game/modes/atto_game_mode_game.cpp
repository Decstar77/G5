#include "atto_game_mode_game.h"
#include "../../shared/atto_colors.h"

namespace atto {
    static i32 YSortEntities( Entity *& a, Entity *& b ) {
        return (i32)( b->pos.y - a->pos.y );
    }

    GameModeType GameModeGame::GetGameModeType() {
        return GameModeType::IN_GAME;
    }

    void GameModeGame::Init( Core * core ) {
        playerCamera = core->CreateDefaultCamera();
    }

    void GameModeGame::UpdateAndRender( Core * core, f32 dt ) {
        DrawContext * worldDraws = core->RenderGetDrawContext( 0 );
        worldDraws->DrawRect( glm::vec2( 0 ), glm::vec2( 100 ), 0.0f, glm::vec4( 1, 0, 0, 1 ) );
        worldDraws->DrawPlane( glm::vec3( 0 ), glm::vec3( 0, 1, 0 ), glm::vec2( 1 ), glm::vec4( 0, 1, 0, 1 ) );
        core->RenderSubmit( worldDraws, true );
    }


    void GameModeGame::Shutdown( Core * core ) {
    }


    Collider Entity::GetSelectionColliderWorld() const {
        Collider c = selectionCollider;
        c.Translate( pos );
        return c;
    }


    Collider Entity::GetCollisionCircleWorld() const {
        Collider c = collisionCollider;
        c.Translate( pos );
        return c;
    }

}
