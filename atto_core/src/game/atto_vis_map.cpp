#include "atto_vis_map.h"
#include "../shared/atto_colors.h"
#include "atto_core.h"

namespace atto {
    void TileMap::PlaceTile( i32 xIndex, i32 yIndex, SpriteResource * sprite, i32 tileXIndex, i32 tileYIndex, i32 flags ) {
        Tile tile = {};
        tile.xIndex = xIndex;
        tile.yIndex = yIndex;
        tile.flatIndex = yIndex * TILE_MAX_X_COUNT + xIndex;
        tile.spriteResource = sprite;
        tile.spriteXIndex = tileXIndex;
        tile.spriteYIndex = tileYIndex;
        tile.flags = flags;

        tile.wsCenter = glm::vec2( xIndex * TILE_SIZE + TILE_SIZE * 0.5f, yIndex * TILE_SIZE + TILE_SIZE * 0.5f );
        tile.wsBounds.min = tile.wsCenter + glm::vec2( -TILE_SIZE / 2.0f, -TILE_SIZE / 2.0f );
        tile.wsBounds.max = tile.wsCenter + glm::vec2( TILE_SIZE / 2.0f, TILE_SIZE / 2.0f );
        tiles[ tile.flatIndex ] = tile;
    }

    void TileMap::FillBorder( SpriteResource * sprite, i32 spriteX, i32 spriteY, i32 flags ) {
        for( i32 yIndex = 0; yIndex < TILE_MAX_Y_COUNT; yIndex++ ) {
            for( i32 xIndex = 0; xIndex < TILE_MAX_X_COUNT; xIndex++ ) {
                if( xIndex == 0 || xIndex == TILE_MAX_X_COUNT - 1 || yIndex == 0 || yIndex == TILE_MAX_Y_COUNT - 1 ) {
                    PlaceTile( xIndex, yIndex, sprite, spriteX, spriteY, flags );
                }
            }
        }
    }

    void TileMap::Fill( SpriteResource * sprite, i32 spriteX, i32 spriteY, i32 flags ) {
        for( i32 yIndex = 0; yIndex < TILE_MAX_Y_COUNT; yIndex++ ) {
            for( i32 xIndex = 0; xIndex < TILE_MAX_X_COUNT; xIndex++ ) {
                PlaceTile( xIndex, yIndex, sprite, spriteX, spriteY, flags );
            }
        }
    }

    void VisMap::VisInitialize( Core * core ) {
        this->core = core;

        SpriteResourceCreateInfo spriteCreateInfo = {};
        spriteCreateInfo.frameXCount = 8;
        spriteCreateInfo.frameYCount = 8;
        spriteCreateInfo.frameCount = spriteCreateInfo.frameXCount * spriteCreateInfo.frameYCount;
        spriteCreateInfo.frameWidth = 8;
        spriteCreateInfo.frameHeight = 8;

        SpriteResource * sprTile = ResourceGetAndCreateSprite( "res/game/tilesets/tileset_sand.png", spriteCreateInfo );
        tileMap.Fill( sprTile, 0, 0, 0 );
        tileMap.FillBorder( sprTile, 1, 0, 0 );
        //for( i32 yIndex = 0; yIndex < TILE_MAX_HEIGHT; yIndex++ ) {
        //    for( i32 xIndex = 0; xIndex < TILE_MAX_WIDTH; xIndex++ ) {
        //        i32 spriteX = Random::Int(0 ,3);
        //        i32 spriteY = Random::Int(0 ,3);
        //        tileMap.PlaceTile( xIndex, yIndex, sprTile, spriteX, spriteY, 0 );
        //    }
        //}
    }

    void VisMap::VisUpdate( f32 dt ) {
        static SpriteResource * sprHoverUnit = ResourceGetAndLoadSprite( "res/game/ents/ui_pick_unit_0_16x16.json" );
        static SpriteResource * sprSelectUnit = ResourceGetAndLoadSprite( "res/game/ents/ui_select_unit_0_16x16.json" );

        DrawContext * tileDrawContext = core->RenderGetSpriteDrawContext( 0 );
        DrawContext * spriteDrawContext = core->RenderGetDrawContext( 1 );
        DrawContext * debugDrawContext = core->RenderGetDrawContext( 7 );
        const glm::vec2 cameraDim = glm::vec2( 640, 360 );
        tileDrawContext->SetCameraDims( cameraDim.x, cameraDim.y );
        spriteDrawContext->SetCameraDims( cameraDim.x, cameraDim.y );
        debugDrawContext->SetCameraDims( cameraDim.x, cameraDim.y );

        const glm::vec2 mousePosPix = core->InputMousePosPixels();
        const glm::vec2 mousePosWorld = spriteDrawContext->ScreenPosToWorldPos( mousePosPix );

        FixedList<SimEntity *, MAX_ENTITY_COUNT > & entities = *MemoryAllocateTransient< FixedList< SimEntity *, MAX_ENTITY_COUNT > >();
        entityPool.GatherActiveObjs( entities );

        const f32 cameraSpeed = 200.0f * dt;
        if ( core->InputKeyDown( KEY_CODE_W ) == true ) {
            cameraPos.y += cameraSpeed;
        }
        if ( core->InputKeyDown( KEY_CODE_S ) == true ) {
            cameraPos.y -= cameraSpeed;
        }
        if ( core->InputKeyDown( KEY_CODE_A ) == true ) {
            cameraPos.x -= cameraSpeed;
        }
        if ( core->InputKeyDown( KEY_CODE_D ) == true ) {
            cameraPos.x += cameraSpeed;
        }

        const glm::vec2 cameraMin = glm::vec2( 0, 0 );
        const glm::vec2 cameraMax = glm::vec2( TILE_MAX_X_COUNT * TILE_SIZE, TILE_MAX_Y_COUNT * TILE_SIZE ) - cameraDim;
        cameraPos = glm::clamp( cameraPos, cameraMin, cameraMax );

        tileDrawContext->SetCameraPos( cameraPos );
        spriteDrawContext->SetCameraPos( cameraPos );
        debugDrawContext->SetCameraPos( cameraPos );

        bool isMouseOverUI = false;
        if ( isMouseOverUI == false && core->InputMouseButtonJustPressed( MOUSE_BUTTON_1 ) == true ) {
            if ( inputMode == InputMode::NONE ) {
                inputMode = InputMode::DRAGGING;
                inputDragStart = mousePosWorld;
                inputDragEnd = mousePosWorld;
                inputDragSelection.Clear();
            }
        }

        if ( inputMode == InputMode::DRAGGING ) {
            inputDragEnd = mousePosWorld;
            if ( core->InputMouseButtonJustReleased( MOUSE_BUTTON_1 ) == true ) {
                inputMode = InputMode::NONE;
                actionBuffer.Request_SelectEntities( playerNumber, inputDragSelection.GetSpan() );
            }
        }

        BoxBounds2D inputDragSelectionBounds = {};
        inputDragSelectionBounds.min = glm::min( inputDragStart, inputDragEnd );
        inputDragSelectionBounds.max = glm::max( inputDragStart, inputDragEnd );

        for ( i32 i = 0; i < TILE_MAX_X_COUNT * TILE_MAX_Y_COUNT; i++ ) {
            Tile * tile = & tileMap.tiles[i];
            if ( tile->spriteResource != nullptr ) {
                //tileDrawContext->DrawSpriteTile( tile->spriteResource, tile->spriteXIndex, tile->spriteYIndex, tile->wsCenter );
            }
        }

        SimEntity * hoveredEntity = nullptr;

        const i32 entityCount = entities.GetCount();
        for( i32 entityIndex = 0; entityIndex < entityCount; entityIndex++ ) {
            SimEntity * simEnt = entities[ entityIndex ];
            VisEntity * visEnt = simEnt->vis;

            glm::vec2 pos = simEnt->pos;
            Collider2D wsCollider = simEnt->ColliderWorldSpace( pos );

            bool hoveredDrag = false;
           
            if ( inputMode == InputMode::NONE ) {
                if ( wsCollider.Contains( mousePosWorld ) == true ) {
                    hoveredEntity = simEnt;
                }
            } else if ( inputMode == InputMode::DRAGGING ) {
                if ( wsCollider.Intersects( inputDragSelectionBounds ) == true ) {
                    inputDragSelection.AddUnique( simEnt->handle );
                    hoveredDrag = true;
                }
            }

//            if ( simEnt->type == EntityType::UNIT_SCOUT ) {
//                glm::vec2 dir = glm::vec2( 0, 0 );
//                bool isMoving = simEnt->posTimeline.GetMovingDirection( dir );
//                if ( isMoving == true ) {
//                    dir = glm::normalize( dir ); // @SPEED: Is there a way to rid the normalize ?
//                    const glm::vec2 upDir = glm::vec2( 0, 1 );
//                    const glm::vec2 downDir = glm::vec2( 0, -1 );
//                    constexpr f32 reqAngle = glm::radians( 45.0f );
//                    if ( glm::acos( glm::dot( dir, upDir ) ) < reqAngle ) {
//                        visEnt->facingDir = FacingDirection::UP;
//                        visEnt->spriteAnimator.SetSpriteIfDifferent( visEnt->spriteBankWalkUp, true );
//                    } else if ( glm::acos( glm::dot( dir, downDir ) ) < reqAngle ) {
//                        visEnt->facingDir = FacingDirection::DOWN;
//                        visEnt->spriteAnimator.SetSpriteIfDifferent( visEnt->spriteBankWalkDown, true );
//                    } else {
//                        visEnt->facingDir = glm::sign( dir.x ) >= 0 ? FacingDirection::RIGHT : FacingDirection::LEFT;
//                        visEnt->spriteAnimator.SetSpriteIfDifferent( visEnt->spriteBankWalkSide, true );
//                    }
//                } else if ( simEnt->unit.state == UnitState::ATTACKING ) {
//                    if ( visEnt->facingDir == FacingDirection::LEFT || visEnt->facingDir == FacingDirection::RIGHT ) {
//                        visEnt->spriteAnimator.SetSpriteIfDifferent( visEnt->spriteBankAttackSide, true );
//                    } else if ( visEnt->facingDir == FacingDirection::UP ) {
//                        visEnt->spriteAnimator.SetSpriteIfDifferent( visEnt->spriteBankAttackUp, true );
//                    } else {
//                        visEnt->spriteAnimator.SetSpriteIfDifferent( visEnt->spriteBankAttackDown, true );
//                    }
//                } else {
//                    if ( visEnt->facingDir == FacingDirection::LEFT || visEnt->facingDir == FacingDirection::RIGHT ) {
//                        visEnt->spriteAnimator.SetSpriteIfDifferent( visEnt->spriteBankIdleSide, true );
//                    } else if ( visEnt->facingDir == FacingDirection::UP ) {
//                        visEnt->spriteAnimator.SetSpriteIfDifferent( visEnt->spriteBankIdleUp, true );
//                    } else {
//                        visEnt->spriteAnimator.SetSpriteIfDifferent( visEnt->spriteBankIdleDown, true );
//                    }
//                }
//            } else if ( simEnt->type == EntityType::STRUCTURE_CITY_CENTER ) {
//
//            }

            visEnt->spriteAnimator.Update( core, dt );
            f32 flipFacing = visEnt->facingDir == FacingDirection::LEFT ? -1.0f : 1.0f;
            //spriteDrawContext->DrawSprite( visEnt->spriteAnimator.sprite, visEnt->spriteAnimator.frameIndex, simEnt->pos, 0, glm::vec2( flipFacing, 1 ), visEnt->spriteAnimator.color );
            spriteDrawContext->DrawSprite( visEnt->spriteAnimator.sprite, visEnt->spriteAnimator.frameIndex, pos, 0, glm::vec2( flipFacing, 1 ), visEnt->spriteAnimator.color );

            if ( simEnt->selectedBy.Contains( playerNumber ) == true ) {
                spriteDrawContext->DrawSprite( sprSelectUnit, 0, pos - glm::vec2( 0, 1 ) );
            } else if ( hoveredDrag == true ) {
                spriteDrawContext->DrawSprite( sprHoverUnit, 0, pos );
            }

            if ( false ) {
                spriteDrawContext->DrawRect( wsCollider.box.min, wsCollider.box.max, glm::vec4( 0.6f ) );
            }

            //spriteDrawContext->DrawCircle( ent->posTimeline.ValueForTime( visTime ), 4 );
        }

        if ( inputMode == InputMode::DRAGGING ) {
            spriteDrawContext->DrawRect( inputDragSelectionBounds.min, inputDragSelectionBounds.max, Colors::BOX_SELECTION_COLOR );
        }

        if( core->InputMouseButtonJustPressed( MouseButton::MOUSE_BUTTON_2 ) == true ) {
            if ( hoveredEntity == nullptr ) {
                actionBuffer.Request_MoveUnit( playerNumber, mousePosWorld );
            } else if ( hoveredEntity->teamNumber != teamNumber ) {
                actionBuffer.Request_AttackUnit( playerNumber, hoveredEntity->handle );
            }
        }

        FontHandle fontHandle = core->ResourceGetFont( "default" );
        SmallString fpsText = StringFormat::Small( "fps=%f", 1.0f / dt );
        debugDrawContext->DrawTextScreen( fontHandle, glm::vec2( 0, 320 ), 20, fpsText.GetCStr() );
        SmallString pingText = StringFormat::Small( "ping=%d", NetworkGetPing() );
        debugDrawContext->DrawTextScreen( fontHandle, glm::vec2( 0, 340 ), 20, pingText.GetCStr() );

        //core->RenderSubmit( tileDrawContext, true );
        core->RenderSubmit( spriteDrawContext, true );
        core->RenderSubmit( debugDrawContext, false );
    }

    VisEntity * VisMap::VisMap_OnSpawnEntity( EntitySpawnCreateInfo createInfo ) {
        VisEntityHandle handle = {};
        VisEntity * visEntity = visEntityPool.Add( handle );
        AssertMsg( visEntity != nullptr, "Spawn Vis Entity is nullptr" );
        if ( visEntity != nullptr ) {
            ZeroStructPtr( visEntity );
            visEntity->handle = handle;
            visEntity->facingDir = FacingDirection::LEFT;

            if ( createInfo.type == EntityType::UNIT_SCOUT ) {
                if ( createInfo.playerNumber.value == 1 ) {
                    visEntity->spriteBankIdleSide = ResourceGetAndLoadSprite( "res/game/ents/units/scout/blue/scout_idle_side.spr.json" );
                    visEntity->spriteBankIdleDown = ResourceGetAndLoadSprite( "res/game/ents/units/scout/blue/scout_idle_down.spr.json" );
                    visEntity->spriteBankIdleUp = ResourceGetAndLoadSprite( "res/game/ents/units/scout/blue/scout_idle_up.spr.json" );
                    visEntity->spriteBankWalkSide = ResourceGetAndLoadSprite( "res/game/ents/units/scout/blue/scout_walk_side.spr.json" );
                    visEntity->spriteBankWalkDown = ResourceGetAndLoadSprite( "res/game/ents/units/scout/blue/scout_walk_down.spr.json" );
                    visEntity->spriteBankWalkUp = ResourceGetAndLoadSprite( "res/game/ents/units/scout/blue/scout_walk_up.spr.json" );
                    visEntity->spriteBankAttackSide = ResourceGetAndLoadSprite( "res/game/ents/units/scout/blue/scout_attack_side.spr.json" );
                    visEntity->spriteBankAttackDown = ResourceGetAndLoadSprite( "res/game/ents/units/scout/blue/scout_attack_down.spr.json" );
                    visEntity->spriteBankAttackUp = ResourceGetAndLoadSprite( "res/game/ents/units/scout/blue/scout_attack_up.spr.json" );
                } else if ( createInfo.playerNumber.value == 2 ) {
                    visEntity->spriteBankIdleSide = ResourceGetAndLoadSprite( "res/game/ents/units/scout/red/scout_idle_side.spr.json" );
                    visEntity->spriteBankIdleDown = ResourceGetAndLoadSprite( "res/game/ents/units/scout/red/scout_idle_down.spr.json" );
                    visEntity->spriteBankIdleUp = ResourceGetAndLoadSprite( "res/game/ents/units/scout/red/scout_idle_up.spr.json" );
                    visEntity->spriteBankWalkSide = ResourceGetAndLoadSprite( "res/game/ents/units/scout/red/scout_walk_side.spr.json" );
                    visEntity->spriteBankWalkDown = ResourceGetAndLoadSprite( "res/game/ents/units/scout/red/scout_walk_down.spr.json" );
                    visEntity->spriteBankWalkUp = ResourceGetAndLoadSprite( "res/game/ents/units/scout/red/scout_walk_up.spr.json" );
                }
                visEntity->spriteAnimator.SetSpriteIfDifferent( visEntity->spriteBankIdleSide, true );
            } else if ( createInfo.type == EntityType::STRUCTURE_CITY_CENTER ) {
                SpriteResourceCreateInfo createInfo = {};
                createInfo.frameCount = 4;
                createInfo.frameXCount = 4;
                createInfo.frameYCount = 1;
                createInfo.frameWidth = 24;
                createInfo.frameHeight = 24;
                createInfo.frameRate = 0;
                createInfo.bakeInAtlas = true;
                SpriteResource * spr = ResourceGetAndCreateSprite( "res/game/ents/structures/sprites/structure_town_center.png", createInfo );
                visEntity->spriteAnimator.SetSpriteIfDifferent( spr, false );
                visEntity->spriteAnimator.frameIndex = 3;
            }
        }

        return visEntity;
    }
}

