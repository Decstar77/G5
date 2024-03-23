#pragma once

#include "atto_vis_map.h"
#include "../../shared/atto_core.h"
#include "../../shared/atto_network.h"
#include "../atto_map_communicator.h"

namespace atto {
   

    //void VisMap::Initialize( Core * core, SimMap * simMap, MapCommunicator * communicator, i32 playerNumber, i32 teamNumber ) {
    //    this->simMap = simMap;
    //    this->communicator = communicator;
    //    localPlayerNumber = playerNumber;
    //    localPlayerTeamNumber = teamNumber;
    //}

    //void VisMap::Render( Core * core, f32 dt ) {
    //    static FontHandle fontHandle = core->ResourceGetFont( "default" );
    //    static TextureResource * background = core->ResourceGetAndLoadTexture( "res/maps/dark_blue.png" );

    //    DrawContext * spriteDrawContext = core->RenderGetDrawContext( 0, true );
    //    DrawContext * backgroundDrawContext = core->RenderGetDrawContext( 1, true );
    //    DrawContext * debugDrawContext = core->RenderGetDrawContext( 2, true );

    //    EntList & entities = *core->MemoryAllocateTransient<EntList>();
    //    simMap->entityPool.GatherActiveObjs( entities );

    //    const f32 cameraSpeed = 20.0f;
    //    if( core->InputKeyDown( KEY_CODE_W ) == true ) {
    //        localCameraPos.y += cameraSpeed;
    //    }
    //    if( core->InputKeyDown( KEY_CODE_S ) == true ) {
    //        localCameraPos.y -= cameraSpeed;
    //    }
    //    if( core->InputKeyDown( KEY_CODE_A ) == true ) {
    //        localCameraPos.x -= cameraSpeed;
    //    }
    //    if( core->InputKeyDown( KEY_CODE_D ) == true ) {
    //        localCameraPos.x += cameraSpeed;
    //    }
    //    
    //    f32 zoomDelta = core->InputMouseWheelDelta();
    //    if( zoomDelta != 0.0f ) {
    //        localCameraZoom -= zoomDelta * 0.1f;
    //        localCameraZoom = glm::clamp( localCameraZoom, 0.0f, 1.0f );

    //        const f32 smallestCameraWidth = 640;
    //        const f32 smallestCameraHeight = 360;
    //        const f32 largestCameraWidth = 1920;
    //        const f32 largestCameraHeight = 1080;
    //        const f32 newCameraWidth = glm::mix( smallestCameraWidth, largestCameraWidth, localCameraZoom );
    //        const f32 newCameraHeight = glm::mix( smallestCameraHeight, largestCameraHeight, localCameraZoom );
    //        const f32 currentCameraWidth = spriteDrawContext->GetCameraWidth();
    //        const f32 currentCameraHeight = spriteDrawContext->GetCameraHeight();

    //        localCameraPos.x -= ( newCameraWidth - currentCameraWidth ) * 0.5f;
    //        localCameraPos.y -= ( newCameraHeight - currentCameraHeight ) * 0.5f;

    //        core->RenderSetCameraDims( newCameraWidth, newCameraHeight );
    //    }

    //    const glm::vec2 mapMin = glm::vec2( 0.0f );
    //    const glm::vec2 mapMax = glm::vec2( 3000.0f ) - glm::vec2( spriteDrawContext->GetCameraWidth(), spriteDrawContext->GetCameraHeight() );
    //    localCameraPos = glm::clamp( localCameraPos, mapMin, mapMax );

    //    spriteDrawContext->SetCameraPos( localCameraPos );
    //    debugDrawContext->SetCameraPos( localCameraPos );

    //    spriteDrawContext->DrawTextureBL( background, glm::vec2( 0, 0 ) );

    //    const glm::vec2 mousePosPix = core->InputMousePosPixels();
    //    const glm::vec2 mousePosWorld = spriteDrawContext->ScreenPosToWorldPos( mousePosPix );
    //    const double currentTime = core->GetTheCurrentTime();

    //    const i32 entityCount = entities.GetCount();
    //    for( i32 entityIndexA = 0; entityIndexA < entityCount; entityIndexA++ ) {
    //        SimEntity * ent = entities[ entityIndexA ];

    //        ent->visPos = glm::mix( ent->visPos, ent->pos, dt * 7.0f );
    //        ent->visOri = glm::mix( ent->visOri, ent->ori, dt * 7.0f );

    //        glm::vec2 drawPos = ent->visPos;
    //        f32 drawOri = ent->visOri;

    //        debugDrawContext->DrawRect( ent->navigator.dest, glm::vec2( 10 ), 0.0f );


    //        if( ent->selectedBy.Contains( localPlayerNumber ) && ent->selectionAnimator.sprite != nullptr ) {
    //            //spriteDrawContext->DrawSprite( ent->selectionAnimator.sprite, ent->selectionAnimator.frameIndex, ent->pos, ent->ori );
    //            spriteDrawContext->DrawSprite( ent->selectionAnimator.sprite, ent->selectionAnimator.frameIndex, drawPos, drawOri );
    //        }

    //        if( ent->spriteAnimator.sprite != nullptr ) {
    //            spriteDrawContext->DrawSprite( ent->spriteAnimator.sprite, ent->spriteAnimator.frameIndex, drawPos, drawOri );
    //        }

    //        if( ent->type == EntityType::UNIT_TEST ) {
    //            Unit & unit = ent->unit;
    //            const i32 turretCount = unit.turrets.GetCount();
    //            for( i32 turretIndex = 0; turretIndex < turretCount; turretIndex++ ) {
    //                UnitTurret & turret = unit.turrets[ turretIndex ];
    //                glm::vec2 worldPos = glm::rotate( turret.pos, ent->ori );

    //            }
    //        }

    //        if( core->InputMouseButtonJustPressed( MOUSE_BUTTON_1 ) == true ) {
    //            Collider2D selectionCollider = ent->GetWorldSelectionCollider();
    //            if( selectionCollider.Contains( mousePosWorld ) ) {
    //                EntHandleList & selection = *core->MemoryAllocateTransient<EntHandleList>();
    //                selection.Add( ent->handle );
    //                communicator->Request_Selection( localPlayerNumber, selection, ENTITY_SELECTION_CHANGE_SET );
    //            }
    //        }
    //    }

    //    if( core->InputMouseButtonJustPressed( MOUSE_BUTTON_2 ) == true ) {
    //        bool inputMade = false;
    //        for( i32 entityIndexA = 0; entityIndexA < entityCount; entityIndexA++ ) {
    //            const SimEntity * ent = entities[ entityIndexA ];
    //            const Collider2D selectionCollider = ent->GetWorldSelectionCollider();
    //            if( selectionCollider.Contains( mousePosWorld ) ) {
    //                if( ent->teamNumber != localPlayerTeamNumber ) {
    //                    // Attack
    //                    communicator->Request_Attack( localPlayerNumber, ent->handle );
    //                } else{
    //                    // Follow
    //                }

    //                inputMade = true;
    //                break;
    //            }
    //        }

    //        if( inputMade == false ) {
    //            communicator->Request_Move( localPlayerNumber, mousePosWorld );
    //        }
    //    }

    //    SmallString s = StringFormat::Small( "ping=%d", (i32)core->NetworkGetPing() );
    //    spriteDrawContext->DrawText2D( fontHandle, glm::vec2( 128, 128 ), 32, s.GetCStr() );
    //    s = StringFormat::Small( "dt=%f", dt );
    //    spriteDrawContext->DrawText2D( fontHandle, glm::vec2( 128, 160 ), 32, s.GetCStr() );
    //    s = StringFormat::Small( "fps=%f", 1.0f / dt );
    //    spriteDrawContext->DrawText2D( fontHandle, glm::vec2( 128, 200 ), 32, s.GetCStr() );

    //    core->RenderSubmit( spriteDrawContext, true );
    //    core->RenderSubmit( backgroundDrawContext, false );
    //    core->RenderSubmit( debugDrawContext, false );
    //}

    //void VisMap::OnSpawnEntity( Core * core, SimEntity * entity ) {
    //    entity->visPos = entity->pos;
    //    entity->visOri = entity->ori;

    //    switch( entity->type ) {
    //        case EntityType::UNIT_TEST:
    //        {
    //            static SpriteResource * mainSprite = core->ResourceGetAndCreateSprite( "res/ents/test/ship.png", 1, 48, 48, 0 );
    //            static SpriteResource * selectionSprite = core->ResourceGetAndCreateSprite( "res/ents/test/ship_selected.png", 1, 48, 48, 0 );
    //            entity->spriteAnimator.SetSpriteIfDifferent( core, mainSprite, false );
    //            entity->selectionAnimator.SetSpriteIfDifferent( core, selectionSprite, false );
    //        } break;
    //    }
    //}





}