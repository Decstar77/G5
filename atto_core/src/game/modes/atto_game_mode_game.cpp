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
        localPlayer = SpawnPlayer( glm::vec3( 0, 0, 3 ) );
    }

    void GameModeGame::UpdateAndRender( Core * core, f32 dt ) {
        EntityUpdateCamera( core, localPlayer );

        DrawContext * worldDraws = core->RenderGetDrawContext( 0 );
        worldDraws->SetCamera( localPlayer->CameraGetViewMatrix(), localPlayer->camera.yfov, localPlayer->camera.zNear, localPlayer->camera.zFar );
        worldDraws->DrawRect( glm::vec2( 0 ), glm::vec2( 100 ), 0.0f, glm::vec4( 1, 0, 0, 1 ) );
        worldDraws->DrawPlane( glm::vec3( 0 ), glm::vec3( 0, 1, 0 ), glm::vec2( 1 ), glm::vec4( 0, 1, 0, 1 ) );
        core->RenderSubmit( worldDraws, true );
    }

    void GameModeGame::Shutdown( Core * core ) {
    }

    Entity * GameModeGame::SpawnEntity( EntityType type ) {
        EntityHandle handle;
        Entity *ent= entityPool.Add( handle );
        if( ent != nullptr ) {
            ZeroStructPtr( ent );
            ent->handle = handle;
        }

        return ent;
    }

    Entity * GameModeGame::SpawnPlayer( glm::vec3 pos ) {
        Entity *ent = SpawnEntity( ENTITY_TYPE_PLAYER );
        if( ent != nullptr ) {
            const f32 YAW = -90.0f;
            const f32 PITCH = 0.0f;
            const f32 SPEED = 2.5f;
            const f32 SENSITIVITY = 0.1f;
            const f32 ZOOM = 45.0f;
            ent->pos = pos;
            ent->camera.yaw = YAW;
            ent->camera.pitch = PITCH;
            ent->camera.movementSpeed = SPEED;
            ent->camera.mouseSensitivity = SENSITIVITY;
            ent->camera.yfov = ZOOM;
            ent->camera.zNear = 0.1f;
            ent->camera.zFar = 250.0f;
            ent->camera.front = glm::vec3( 0.0f, 0.0f, -1.0f );
            ent->camera.up = glm::vec3( 0.0f, 1.0f, 0.0f );
            ent->hasCollision = true;
            ent->collisionCollider.type = COLLIDER_TYPE_SHPERE;
            ent->collisionCollider.sphere.r = 1.0f;
            ent->collisionCollider.sphere.c = ent->pos;
        }

        return ent;
    }

    void GameModeGame::EntityUpdateCamera( Core * core, Entity * ent ) {
        if( core->InputMouseButtonJustPressed( MOUSE_BUTTON_2 ) == true ) {
            core->InputDisableMouse();
            return;
        }

        if( core->InputMouseButtonJustReleased( MOUSE_BUTTON_2 ) == true ) {
            core->InputEnableMouse();
            return;
        }

        if( core->InputMouseButtonDown( MOUSE_BUTTON_2 ) == true ) {
            f32 v = ent->camera.movementSpeed * core->GetDeltaTime();
            if( core->InputKeyDown( KEY_CODE_W ) ) {
                ent->pos += ent->camera.front * v;
            }
            if( core->InputKeyDown( KEY_CODE_S ) ) {
                ent->pos -= ent->camera.front * v;
            }
            if( core->InputKeyDown( KEY_CODE_A ) ) {
                ent->pos -= ent->camera.right * v;
            }
            if( core->InputKeyDown( KEY_CODE_D ) ) {
                ent->pos += ent->camera.right * v;
            }
            if( core->InputKeyDown( KEY_CODE_SPACE ) ) {
                ent->pos.y += v;
            }
            if( core->InputKeyDown( KEY_CODE_LEFT_CONTROL ) ) {
                ent->pos.y -= v;
            }

            glm::vec2 mouseDelta = core->InputMouseDeltaPixels();
            f32 xoffset = mouseDelta.x * ent->camera.mouseSensitivity;
            f32 yoffset = mouseDelta.y * ent->camera.mouseSensitivity;
            ent->camera.yaw += xoffset;
            ent->camera.pitch += yoffset;
            if( ent->camera.pitch > 89.0f ) {
                ent->camera.pitch = 89.0f;
            }
            if( ent->camera.pitch < -89.0f ) {
                ent->camera.pitch = -89.0f;
            }

            glm::vec3 front;
            front.x = cos( glm::radians( ent->camera.yaw ) ) * cos( glm::radians( ent->camera.pitch ) );
            front.y = sin( glm::radians( ent->camera.pitch ) );
            front.z = sin( glm::radians( ent->camera.yaw ) ) * cos( glm::radians( ent->camera.pitch ) );
            ent->camera.front = glm::normalize( front );
            ent->camera.right = glm::normalize( glm::cross( ent->camera.front, glm::vec3( 0, 1, 0 ) ) );
            ent->camera.up = glm::normalize( glm::cross( ent->camera.right, ent->camera.front ) );
        }
    }

    glm::mat4 Entity::CameraGetViewMatrix() const {
        return glm::lookAt( pos, pos + camera.front, camera.up );
    }

}
