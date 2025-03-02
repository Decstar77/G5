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

        tile.wsCenterFl = glm::vec2( xIndex * TILE_SIZE + TILE_SIZE * 0.5f, yIndex * TILE_SIZE + TILE_SIZE * 0.5f );
        tile.wsBoundsFl.min = tile.wsCenterFl + glm::vec2( -TILE_SIZE / 2.0f, -TILE_SIZE / 2.0f );
        tile.wsBoundsFl.max = tile.wsCenterFl + glm::vec2( TILE_SIZE / 2.0f, TILE_SIZE / 2.0f );

        tile.wsCenterFp = Fp2( xIndex * TILE_SIZE + TILE_SIZE / 2, yIndex * TILE_SIZE + TILE_SIZE / 2 );
        tile.wsBoundsFp.min = tile.wsCenterFp + Fp2( -TILE_SIZE / 2, -TILE_SIZE / 2 );
        tile.wsBoundsFp.max = tile.wsCenterFp + Fp2( TILE_SIZE / 2, TILE_SIZE / 2 );

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

    void TileMap::DebugDraw( DrawContext * drawContext ) {
        glm::vec2 bl = glm::vec2( 0, 0 );
        glm::vec2 tr = glm::vec2( TILE_MAX_X_COUNT * TILE_SIZE, TILE_MAX_Y_COUNT * TILE_SIZE );

        for ( i32 x = 0; x < TILE_MAX_X_COUNT; x++ ) {
            f32 xp = (f32)(x * TILE_SIZE);
            drawContext->DrawLine( glm::vec2( xp, 0 ), glm::vec2( xp, TILE_MAX_Y_COUNT * TILE_SIZE ), 1 );
        }

        for ( i32 y = 0; y < TILE_MAX_Y_COUNT; y++ ) {
            f32 yp = (f32)(y * TILE_SIZE);
            drawContext->DrawLine( glm::vec2( 0, yp ), glm::vec2( TILE_MAX_X_COUNT * TILE_SIZE, yp ), 1 );
        }
    }

    void TileMap::PosToTileIndex( fp2 p, i32 & xi, i32 & yi ) {
        fp x = p.x / Fp( TILE_SIZE );
        fp y = p.y / Fp( TILE_SIZE );
        x = FpRound( x );
        y = FpRound( y );
        xi = ToInt( x );
        yi = ToInt( y );
    }

    fp2 TileMap::PosToTileBL( fp2 p ) {
        i32 xi = 0;
        i32 yi = 0;
        PosToTileIndex( p, xi, yi );
        return Fp2( xi * TILE_SIZE, yi * TILE_SIZE );
    }

    void TileMap::MarkTilesBL( i32 xIndex, i32 yIndex, i32 xCount, i32 yCount, i32 flags ) {
        Assert( xIndex >= 0 && xIndex < TILE_MAX_X_COUNT );
        Assert( yIndex >= 0 && yIndex < TILE_MAX_Y_COUNT );
        Assert( xIndex + xCount >= 0 && xIndex + xCount < TILE_MAX_X_COUNT );
        Assert( yIndex + yCount >= 0 && yIndex + yCount < TILE_MAX_Y_COUNT );

        for( i32 y = yIndex; y < yIndex + yCount; y++ ) {
            for( i32 x = xIndex; x < xIndex + xCount; x++ ) {
                i32 flatIndex = y * TILE_MAX_X_COUNT + x;
                tiles[ flatIndex ].flags |= flags;
            }
        }
    }

    bool TileMap::ContainsFlag( i32 xIndex, i32 yIndex, i32 xCount, i32 yCount, i32 flag ) {
        Assert( xIndex >= 0 && xIndex < TILE_MAX_X_COUNT );
        Assert( yIndex >= 0 && yIndex < TILE_MAX_Y_COUNT );
        Assert( xIndex + xCount >= 0 && xIndex + xCount < TILE_MAX_X_COUNT );
        Assert( yIndex + yCount >= 0 && yIndex + yCount < TILE_MAX_Y_COUNT );

        for( i32 y = yIndex; y < yIndex + yCount; y++ ) {
            for( i32 x = xIndex; x < xIndex + xCount; x++ ) {
                i32 flatIndex = y * TILE_MAX_X_COUNT + x;
                if ( EnumContainsFlag( tiles[ flatIndex ].flags, flag ) == true ) {
                    return true;
                }
            }
        }

        return false;
    }

    TileInterval TileMap::IntervalForCollider( FpCollider collider, fp2 pos ) {
        Assert( collider.type == ColliderType::COLLIDER_TYPE_AXIS_BOX );

        // @NOTE: We have to do this size nonsense because of drawing at centers rather than BL's
        //      : For instance a 24x24 sprite has 12 halfDim. 12 / 8 = 1.5 so without ceil we loose
        //      : a full tile.
        const fp2 halfSize = FpAxisBoxGetSize( collider.box ) / Fp( 2 );
        const i32 sizeX = ToInt( FpCeil( halfSize.x / Fp( TILE_SIZE ) ) ) * 2;
        const i32 sizeY = ToInt( FpCeil( halfSize.y / Fp( TILE_SIZE ) ) ) * 2;
        i32 xi = 0;
        i32 yi = 0;
        PosToTileIndex( pos, xi, yi );

        TileInterval result = {};
        result.xIndex = xi - sizeX / 2;
        result.yIndex = yi - sizeY / 2;
        result.xCount = sizeX; 
        result.yCount = sizeY;

        return result;
    }

    static GUIGamePanel GuiCreatePanel( glm::vec2 p ) {
        GUIGamePanel panel = {};
        panel.activeIndex = -1;
        panel.pos = p;
        panel.texture = ResourceGetAndLoadSprite( "res/game/ents/ui/ui_block.spr.json" );
        const glm::vec2 baseTopLeft = glm::vec2( 7, 5 );
        for ( i32 y = 0; y < 3; y++ ) {
            for ( i32 x = 0; x < 3; x++ ) {
                i32 flatIndex = y * 3 + x;
                panel.imageBls[ flatIndex ] = p + baseTopLeft + glm::vec2( 11 * x, 11 * y );
                panel.imageColors[ flatIndex ] = glm::vec4( 1, 1, 1, 1 );
            }
        }

        return panel;
    }

    static GUIGamePanelSmol GuiCreatePanelSmol( glm::vec2 p ) {
        GUIGamePanelSmol panel = {};
        panel.pos = p;
        panel.texture = ResourceGetAndLoadSprite( "res/game/ents/ui/ui_block_smol.spr.json" );
        panel.leftBl = p + glm::vec2( 4, 2 );
        return panel;
    }

    void VisMap::VisInitialize( Core * core ) {
        this->core = core;
 
        //{
        //    UIGamePanel panel = UI_CreateBlock( glm::vec2( 276, 1 ) );
        //    guiLayer->DrawTextureBL( panel.texture->textureResource, panel.pos );
        //    for ( int i = 0; i < 9; i++ ) {
        //        guiLayer->DrawRect( panel.bls[i], panel.bls[i] + glm::vec2( 8, 8 ) );
        //    }
        //}

        Gui_BlockSelection = ResourceGetAndLoadSprite( "res/game/ents/ui/ui_pick_block_14x14.spr.json" );
        guiNothingPanel = GuiCreatePanel( glm::vec2( 276, 1 ) );
        guiCurrentPanel = &guiNothingPanel;

        guiTownCenterPanel = GuiCreatePanel( glm::vec2( 276, 1 ) );
        guiTownCenterPanel.images[ 6 ] = ResourceGetAndLoadSprite( "res/game/ents/structures/town_center/town_center_icon.spr.json" );
        guiTownCenterPanel.images[ 7 ] = ResourceGetAndLoadSprite( "res/game/ents/structures/smol_reactor/smol_reactor_icon.spr.json" );
        guiTownCenterPanel.images[ 8 ] = ResourceGetAndLoadSprite( "res/game/ents/units/scout/scout_icon.spr.json" );
        guiTownCenterUnlocksPanel = GuiCreatePanel( glm::vec2( 276, 42 ) );
        guiTownCenterUnlocksPanel.texture =  ResourceGetAndLoadSprite( "res/game/ents/ui/ui_block_unlock.spr.json" );
        guiTownCenterUnlocksPanel.images[ 6 ] =  ResourceGetAndLoadSprite( "res/game/ents/structures/town_center/town_center_icon.spr.json" );
        guiTownCenterUnlocksPanel.imageColors[ 6 ] = glm::vec4( 1, 1, 1, 0.5f );

        guiUnitPanel = GuiCreatePanel( glm::vec2( 276, 1 ) );
        guiUnitPanel.images[ 6 ] = ResourceGetAndLoadSprite( "res/game/ents/units/icons/action_icon_move.spr.json" );
        guiUnitPanel.images[ 7 ] = ResourceGetAndLoadSprite( "res/game/ents/units/icons/action_icon_stop.spr.json" );
        guiUnitPanel.images[ 8 ] = ResourceGetAndLoadSprite( "res/game/ents/units/icons/action_icon_hold.spr.json" );
        guiUnitPanel.images[ 3 ] = ResourceGetAndLoadSprite( "res/game/ents/units/icons/action_icon_patrol.spr.json" );
        guiUnitPanel.images[ 4 ] = ResourceGetAndLoadSprite( "res/game/ents/units/icons/action_icon_attack.spr.json" );
    }

    void VisMap::VisUpdate( f32 dt ) {
        DrawContext * tileLayer = core->RenderGetSpriteDrawContext( 0 );
        DrawContext * spriteLayer0 = core->RenderGetDrawContext( 1 );
        DrawContext * spriteLayer1 = core->RenderGetDrawContext( 2 );
        DrawContext * guiLayer = core->RenderGetDrawContext( 3 );
        DrawContext * debugLayer = core->RenderGetDrawContext( 7 );

        // @NOTE: World Camera Dims
        const glm::vec2 cameraDim = glm::vec2( 640, 360 );
        tileLayer->SetCameraDims( cameraDim.x, cameraDim.y );
        spriteLayer0->SetCameraDims( cameraDim.x, cameraDim.y );
        spriteLayer1->SetCameraDims( cameraDim.x, cameraDim.y );
        debugLayer->SetCameraDims( cameraDim.x, cameraDim.y );

        const glm::vec2 mousePosPix = core->InputMousePosPixels();
        const glm::vec2 mousePosWorld = spriteLayer0->ScreenPosToWorldPos( mousePosPix );
        const fp2 mousePosWorldFp = Fp2( mousePosWorld );

        // @NOTE: Gui Camera Dims
        guiLayer->SetCameraDims( 320, 180 );
        bool isMouseOverUI = false;
        const glm::vec2 mousePosGui = guiLayer->ScreenPosToWorldPos( mousePosPix );
        guiLayer->DrawTextureBL( ResourceGetAndCreateTexture( "res/game/gui/ui_test_04.png" ), glm::vec2( 0 ) );

        if ( guiCurrentPanel != nullptr ) {
            OnGUILeftPanelUpdate();
            isMouseOverUI = OnGuiPanelDraw( guiCurrentPanel, guiLayer, mousePosGui );
        }

        SmallString guiText = StringFormat::Small( "%d", playerMonies[ playerNumber.value - 1 ].power );
        guiLayer->DrawTextCam( ResourceGetFont( "bandwidth" ), glm::vec2( 40, 57 ), 8, guiText.GetCStr(), TextAlignment_H::FONS_ALIGN_RIGHT );
        guiText = StringFormat::Small( "%d", playerMonies[ playerNumber.value - 1 ].cash );
        guiLayer->DrawTextCam( ResourceGetFont( "bandwidth" ), glm::vec2( 40, 43 ), 8, guiText.GetCStr(), TextAlignment_H::FONS_ALIGN_RIGHT );

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

        tileLayer->SetCameraPos( cameraPos );
        spriteLayer0->SetCameraPos( cameraPos );
        spriteLayer1->SetCameraPos( cameraPos );
        debugLayer->SetCameraPos( cameraPos );

        if ( isMouseOverUI == false && core->InputMouseButtonJustPressed( MOUSE_BUTTON_1 ) == true ) {
            if ( inputMode == InputMode::NONE ) {
                inputMode = InputMode::DRAGGING;
                inputDragStart = mousePosWorld;
                inputDragEnd = mousePosWorld;
                inputDragSelection.Clear();
            }
        }

        for ( i32 i = 0; i < TILE_MAX_X_COUNT * TILE_MAX_Y_COUNT; i++ ) {
            Tile * tile = & tileMap.tiles[i];
            if ( tile->spriteResource != nullptr ) {
                //tileLayer->DrawSpriteTile( tile->spriteResource, tile->spriteXIndex, tile->spriteYIndex, tile->wsCenterFl );
            }
            if ( false && EnumHasFlag( tile->flags, TILE_FLAG_UNWALKABLE ) ) {
                glm::vec2 wMin = tile->wsBoundsFl.min + glm::vec2( 1 );
                glm::vec2 wMax = tile->wsBoundsFl.max - glm::vec2( 1 );
                debugLayer->DrawRect( wMin, wMax, glm::vec4( 0.8f, 0.2f, 0.2f, 0.76f ) );
            }
        }
        //tileMap.DebugDraw( debugLayer );

        BoxBounds2D inputDragSelectionBounds = {};
        inputDragSelectionBounds.min = glm::min( inputDragStart, inputDragEnd );
        inputDragSelectionBounds.max = glm::max( inputDragStart, inputDragEnd );

        SimEntity * hoveredEntity = nullptr;

        const i32 entityCount = entities.GetCount();
        for( i32 entityIndex = 0; entityIndex < entityCount; entityIndex++ ) {
            SimEntity * ent = entities[ entityIndex ];

            ent->posTime += dt;
            f32 ptime = glm::clamp( ent->posTime / tickTime, 0.0f, 1.0f );
            glm::vec2 pos = glm::mix( ToVec2( ent->lastPos ), ToVec2( ent->pos ), ptime );
            Collider2D wsCollider = ent->ColliderFlWorldSpace( pos );

            bool isHovered = false;
            if ( inputMode == InputMode::NONE ) {
                if ( wsCollider.Contains( mousePosWorld ) == true ) {
                    hoveredEntity = ent;
                    isHovered = true;
                }
            } else if ( inputMode == InputMode::DRAGGING ) {
                if ( wsCollider.Intersects( inputDragSelectionBounds ) == true ) {
                    inputDragSelection.AddUnique( ent->handle );
                    isHovered = true;
                }
            }

            if ( ent->type == EntityType::UNIT_SCOUT ) {
                glm::vec2 dir = ToVec2( ent->pos ) - ToVec2( ent->lastPos );
                bool isMoving = dir != glm::vec2( 0, 0 );// @TODO: EPSILON!!!
                if ( isMoving == true ) {
                    dir = glm::normalize( dir ); // @SPEED: Is there a way to rid the normalize ?
                    const glm::vec2 upDir = glm::vec2( 0, 1 );
                    const glm::vec2 downDir = glm::vec2( 0, -1 );
                    constexpr f32 reqAngle = glm::radians( 45.0f );
                    if ( glm::acos( glm::dot( dir, upDir ) ) < reqAngle ) {
                        ent->visFacingDir = FacingDirection::UP;
                        ent->spriteAnimator.SetSpriteIfDifferent( ent->sprWalkUp, true );
                    } else if ( glm::acos( glm::dot( dir, downDir ) ) < reqAngle ) {
                        ent->visFacingDir = FacingDirection::DOWN;
                        ent->spriteAnimator.SetSpriteIfDifferent( ent->sprWalkDown, true );
                    } else {
                        ent->visFacingDir = glm::sign( dir.x ) >= 0 ? FacingDirection::RIGHT : FacingDirection::LEFT;
                        ent->spriteAnimator.SetSpriteIfDifferent( ent->sprWalkSide, true );
                    }
                } else if ( ent->unit.state == UnitState::ATTACKING ) {
                    //if ( ent->visFacingDir == FacingDirection::LEFT || ent->visFacingDir == FacingDirection::RIGHT ) {
                    //    ent->spriteAnimator.SetSpriteIfDifferent( ent->sprAttackSide, false );
                    //} else if ( ent->visFacingDir == FacingDirection::UP ) {
                    //    ent->spriteAnimator.SetSpriteIfDifferent( ent->sprAttackUp, false );
                    //} else {
                    //    ent->spriteAnimator.SetSpriteIfDifferent( ent->sprAttackDown, false );
                    //}
                } else {
                    if ( ent->visFacingDir == FacingDirection::LEFT || ent->visFacingDir == FacingDirection::RIGHT ) {
                        ent->spriteAnimator.SetSpriteIfDifferent( ent->sprIdleSide, true );
                    } else if ( ent->visFacingDir == FacingDirection::UP ) {
                        ent->spriteAnimator.SetSpriteIfDifferent( ent->sprIdleUp, true );
                    } else {
                        ent->spriteAnimator.SetSpriteIfDifferent( ent->sprIdleDown, true );
                    }
                }
            } else if ( ent->type == EntityType::STRUCTURE_CITY_CENTER ) {

            }

            ent->spriteAnimator.Update( core, dt );
            f32 flipFacing = ent->visFacingDir == FacingDirection::LEFT ? -1.0f : 1.0f;
            spriteLayer0->DrawSprite( ent->spriteAnimator.sprite, ent->spriteAnimator.frameIndex, pos, 0, glm::vec2( flipFacing, 1 ), ent->spriteAnimator.color );

            if ( isHovered == true || ent->selectedBy.Contains( playerNumber ) == true ) {
                spriteLayer0->DrawSprite( ent->sprSelection, 0, pos + ent->spriteSelectionOffset );
            }

            if ( false ) {
                spriteLayer0->DrawRect( wsCollider.box.min, wsCollider.box.max, glm::vec4( 0.6f ) );
            }
        }

        if ( inputMode == InputMode::DRAGGING ) {
            inputDragEnd = mousePosWorld;
            spriteLayer1->DrawRect( inputDragSelectionBounds.min, inputDragSelectionBounds.max, Colors::BOX_SELECTION_COLOR );
            if ( core->InputMouseButtonJustReleased( MOUSE_BUTTON_1 ) == true ) {
                inputMode = InputMode::NONE;
                actionBuffer.Request_SelectEntities( playerNumber, inputDragSelection.GetSpan() );

                const i32 newSelectionCount = inputDragSelection.GetCount();
                currentSelection.Clear();
                guiCurrentPanel = &guiNothingPanel;
                for ( i32 selectionIndex = 0; selectionIndex < newSelectionCount; selectionIndex++ ) {
                    EntityHandle handle = inputDragSelection[ selectionIndex ];
                    currentSelection.Add( handle );
                    if ( selectionIndex == 0 ) {
                        SimEntity * activeEnt = entityPool.Get( inputDragSelection[ selectionIndex ] );
                        if ( activeEnt != nullptr && IsUnitType( activeEnt->type ) ) {
                            core->AudioPlayRandomFromGroup( activeEnt->sndOnSelect );
                            guiCurrentPanel = &guiUnitPanel;
                        } else if ( activeEnt != nullptr && activeEnt->type == EntityType::STRUCTURE_CITY_CENTER ) {
                            guiCurrentPanel = &guiTownCenterPanel;
                        }
                    }
                }
            }
        } else if ( inputMode == InputMode::PLACING_STRUCTURE && isMouseOverUI == false ) {
            Assert( inputSprite != nullptr );
            fp2 posFp = tileMap.PosToTileBL( mousePosWorldFp );
            glm::vec2 pos = ToVec2( posFp );
            // @TODO: TILE APRIN 6
            bool canPlace = SimCanPlaceStructure( inputPlacingBuildingType, posFp );
            glm::vec4 col = canPlace ?  glm::vec4( 1, 1, 1, 0.55f ) : glm::vec4( 2, 0.55f, 0.55f, 0.77f );
            spriteLayer1->DrawSprite( inputSprite, 3, pos, 0, glm::vec2( 1 ), col );
            if ( core->InputMouseButtonJustReleased( MOUSE_BUTTON_1 ) == true ) {
                if ( canPlace == true ) {
                    // @TODO: Play sound
                    inputMode = InputMode::NONE;
                    actionBuffer.Request_PlaceStructure( playerNumber, inputPlacingBuildingType, posFp );
                    inputPlacingBuildingType = EntityType::INVALID;
                } else {
                    // @TODO: Play sound
                }
            } else if ( core->InputMouseButtonJustReleased( MOUSE_BUTTON_2 ) == true ) {
                inputMode = InputMode::NONE;
                inputPlacingBuildingType = EntityType::INVALID;
            }
        }

        if( core->InputMouseButtonJustPressed( MouseButton::MOUSE_BUTTON_2 ) == true ) {
            if ( hoveredEntity == nullptr ) {
                const i32 selectionCount = currentSelection.GetCount();
                if ( selectionCount > 0 ) {
                    SimEntity * activeEnt = entityPool.Get( currentSelection[ 0 ] );
                    if ( activeEnt != nullptr && IsUnitType( activeEnt->type ) ) {
                        core->AudioPlayRandomFromGroup( activeEnt->sndOnMove );
                    }
                }

                actionBuffer.Request_MoveUnit( playerNumber, mousePosWorldFp );
            } else if ( hoveredEntity->teamNumber != teamNumber ) {
                const i32 selectionCount = currentSelection.GetCount();
                if ( selectionCount > 0 ) {
                    SimEntity * activeEnt = entityPool.Get( currentSelection[ 0 ] );
                    if ( activeEnt != nullptr && IsUnitType( activeEnt->type ) ) {
                        core->AudioPlayRandomFromGroup( activeEnt->sndOnAttack );
                    }
                }

                actionBuffer.Request_AttackUnit( playerNumber, hoveredEntity->handle );
            }
        }

        FontHandle fontHandle = core->ResourceGetFont( "default" );
        SmallString fpsText = StringFormat::Small( "fps=%f", 1.0f / dt );
        debugLayer->DrawTextScreen( fontHandle, glm::vec2( 0, core->RenderGetMainSurfaceHeight() - 20 ), 20, fpsText.GetCStr() );
        SmallString pingText = StringFormat::Small( "ping=%d", NetworkGetPing() );
        debugLayer->DrawTextScreen( fontHandle, glm::vec2( 0, core->RenderGetMainSurfaceHeight() - 40 ), 20, pingText.GetCStr() );

        //core->RenderSubmit( tileLayer, true );
        core->RenderSubmit( spriteLayer0, true );
        core->RenderSubmit( spriteLayer1, false );
        core->RenderSubmit( guiLayer, false );
        core->RenderSubmit( debugLayer, false );
    }

    void VisMap::VisMap_OnSpawnEntity( SimEntity * visEntity, EntitySpawnCreateInfo createInfo ) {
        visEntity->visFacingDir = FacingDirection::RIGHT;
        if ( createInfo.type == EntityType::UNIT_SCOUT ) {
            //if ( createInfo.playerNumber.value == 1 ) {
            //    visEntity->sprIdleSide = ResourceGetAndLoadSprite( "res/game/ents/units/scout/blue/scout_idle_side.spr.json" );
            //    visEntity->sprIdleDown = ResourceGetAndLoadSprite( "res/game/ents/units/scout/blue/scout_idle_down.spr.json" );
            //    visEntity->sprIdleUp = ResourceGetAndLoadSprite( "res/game/ents/units/scout/blue/scout_idle_up.spr.json" );
            //    visEntity->sprWalkSide = ResourceGetAndLoadSprite( "res/game/ents/units/scout/blue/scout_walk_side.spr.json" );
            //    visEntity->sprWalkDown = ResourceGetAndLoadSprite( "res/game/ents/units/scout/blue/scout_walk_down.spr.json" );
            //    visEntity->sprWalkUp = ResourceGetAndLoadSprite( "res/game/ents/units/scout/blue/scout_walk_up.spr.json" );
            //    visEntity->sprAttackSide = ResourceGetAndLoadSprite( "res/game/ents/units/scout/blue/scout_attack_side.spr.json" );
            //    visEntity->sprAttackDown = ResourceGetAndLoadSprite( "res/game/ents/units/scout/blue/scout_attack_down.spr.json" );
            //    visEntity->sprAttackUp = ResourceGetAndLoadSprite( "res/game/ents/units/scout/blue/scout_attack_up.spr.json" );
            //} else if ( createInfo.playerNumber.value == 2 ) {
            //    visEntity->sprIdleSide = ResourceGetAndLoadSprite( "res/game/ents/units/scout/red/scout_idle_side.spr.json" );
            //    visEntity->sprIdleDown = ResourceGetAndLoadSprite( "res/game/ents/units/scout/red/scout_idle_down.spr.json" );
            //    visEntity->sprIdleUp = ResourceGetAndLoadSprite( "res/game/ents/units/scout/red/scout_idle_up.spr.json" );
            //    visEntity->sprWalkSide = ResourceGetAndLoadSprite( "res/game/ents/units/scout/red/scout_walk_side.spr.json" );
            //    visEntity->sprWalkDown = ResourceGetAndLoadSprite( "res/game/ents/units/scout/red/scout_walk_down.spr.json" );
            //    visEntity->sprWalkUp = ResourceGetAndLoadSprite( "res/game/ents/units/scout/red/scout_walk_up.spr.json" );
            //}
            //visEntity->sprSelection = ResourceGetAndLoadSprite( "res/game/ents/ui/ui_select_unit_16x16.spr.json" );
            //visEntity->spriteSelectionOffset = glm::vec2( 0, -1 );
            visEntity->spriteAnimator.SetSpriteIfDifferent( visEntity->sprIdleSide, true );
            //
            //static bool doOnce = false;
            //static AudioGroupResource * sndMoveGroup = nullptr;
            //static AudioGroupResource * sndSelectGroup = nullptr;
            //static AudioGroupResource * sndAttackGroup = nullptr;
            //
            //if ( doOnce == false ) {
            //    doOnce = true;
            //    AudioGroupResourceCreateInfo group = {};
            //    group.maxInstances = 2;
            //    group.stealMode = AudioStealMode::OLDEST;
            //    group.minTimeToPassForAnotherSubmission = 0.5f;
            //    sndMoveGroup = ResourceGetAndCreateAudioGroup( "res/game/ents/units/scout/scout_move.snd.json", &group );
            //    sndMoveGroup->sounds.SetCount( 4 );
            //    sndMoveGroup->sounds[ 0 ] = ResourceGetAndCreateAudio2D( "res/game/ents/units/scout/sounds/scout_move_1.wav" );
            //    sndMoveGroup->sounds[ 1 ] = ResourceGetAndCreateAudio2D( "res/game/ents/units/scout/sounds/scout_move_2.wav" );
            //    sndMoveGroup->sounds[ 2 ] = ResourceGetAndCreateAudio2D( "res/game/ents/units/scout/sounds/scout_move_3.wav" );
            //    sndMoveGroup->sounds[ 3 ] = ResourceGetAndCreateAudio2D( "res/game/ents/units/scout/sounds/scout_move_4.wav" );
            //
            //    sndSelectGroup = ResourceGetAndCreateAudioGroup( "res/game/ents/units/scout/scout_select.snd.json", &group );
            //    sndSelectGroup->sounds.SetCount( 4 );
            //    sndSelectGroup->sounds[ 0 ] = ResourceGetAndCreateAudio2D( "res/game/ents/units/scout/sounds/scout_sir_1.wav" );
            //    sndSelectGroup->sounds[ 1 ] = ResourceGetAndCreateAudio2D( "res/game/ents/units/scout/sounds/scout_sir_2.wav" );
            //    sndSelectGroup->sounds[ 2 ] = ResourceGetAndCreateAudio2D( "res/game/ents/units/scout/sounds/scout_sir_3.wav" );
            //    sndSelectGroup->sounds[ 3 ] = ResourceGetAndCreateAudio2D( "res/game/ents/units/scout/sounds/scout_sir_4.wav" );
            //
            //    sndAttackGroup = ResourceGetAndCreateAudioGroup( "res/game/ents/units/scout/scout_attack.snd.json", &group );
            //    sndAttackGroup->sounds.SetCount( 3 );
            //    sndAttackGroup->sounds[ 0 ] = ResourceGetAndCreateAudio2D( "res/game/ents/units/scout/sounds/scout_enemies_1.wav" );
            //    sndAttackGroup->sounds[ 1 ] = ResourceGetAndCreateAudio2D( "res/game/ents/units/scout/sounds/scout_enemies_2.wav" );
            //    sndAttackGroup->sounds[ 2 ] = ResourceGetAndCreateAudio2D( "res/game/ents/units/scout/sounds/scout_enemies_3.wav" );
            //
            //    //ResourceWriteTextRefl( sndMoveGroup, "res/game/ents/units/scout/scout_move.snd.json" );
            //    //ResourceWriteTextRefl( sndSelectGroup, "res/game/ents/units/scout/scout_select.snd.json" );
            //    //ResourceWriteTextRefl( sndAttackGroup, "res/game/ents/units/scout/scout_attack.snd.json" );
            //}

            //visEntity->sndOnMove = sndMoveGroup;
            //visEntity->sndOnSelect = sndSelectGroup;
            //visEntity->sndOnAttack = sndAttackGroup;

            //ResourceWriteTextRefl( visEntity, "res/game/ents/units/scout/scout_attack.ent.json" );
        }
        else if( createInfo.type == EntityType::STRUCTURE_CITY_CENTER ) {
            visEntity->spriteAnimator.SetSpriteIfDifferent( visEntity->sprIdleSide, false );
            visEntity->spriteAnimator.frameIndex = 3;
        }
        else if( createInfo.type == EntityType::STRUCTURE_SMOL_REACTOR ) {
            visEntity->spriteAnimator.SetSpriteIfDifferent( visEntity->sprIdleSide, false );
            visEntity->spriteAnimator.frameIndex = 3;
        }
    }

    bool VisMap::OnGuiPanelDrawSmol( GUIGamePanelSmol * panel, DrawContext * guiLayer, glm::vec2 mousePosGui ) {
        bool isMouseOverUI = false;
        guiLayer->DrawTextureBL( panel->texture->textureResource, panel->pos );
        if (panel->leftImage != nullptr ) {
            guiLayer->DrawTextureBL( panel->leftImage->textureResource, panel->leftBl );
        }
        return isMouseOverUI;
    }

    bool VisMap::OnGuiPanelDraw( GUIGamePanel * panel, DrawContext * guiLayer, glm::vec2 mousePosGui ) {
        bool isMouseOverUI = false;
        guiLayer->DrawTextureBL( panel->texture->textureResource, panel->pos );
        for ( int i = 0; i < 9; i++ ) {
            SpriteResource * icon = *panel->images.Get(i);
            if ( icon != nullptr ) {
                glm::vec4 col = panel->imageColors[ i ];
                guiLayer->DrawTextureBL( icon->textureResource, panel->imageBls[i], glm::vec2( 1 ), col );
                BoxBounds2D bb = {};
                bb.min = panel->imageBls[ i ];
                bb.max = panel->imageBls[ i ] + glm::vec2( 8, 8 );
                if ( bb.Contains( mousePosGui ) == true ) {
                    guiLayer->DrawTextureBL( Gui_BlockSelection->textureResource, panel->imageBls[i] - glm::vec2( 3 ) );
                    isMouseOverUI = true;
                    OnGUIPanelHoverd( guiLayer, i );
                    if ( core->InputMouseButtonJustPressed( MouseButton::MOUSE_BUTTON_1 ) == true ) {
                        OnGUIPanelClicked( guiLayer, i );
                    }
                }

                if ( panel->activeIndex == i ) {
                    guiLayer->DrawTextureBL( Gui_BlockSelection->textureResource, panel->imageBls[i] - glm::vec2( 3 ) );
                }
            }
        }
        return isMouseOverUI;
    }

    void VisMap::OnGUILeftPanelUpdate() {
        if ( inputMode != InputMode::PLACING_STRUCTURE ) {
            guiTownCenterPanel.activeIndex = - 1;
        }
    }

    void VisMap::OnGUIPanelHoverd( DrawContext * guiLayer, i32 idx ) {
        if ( guiCurrentPanel == &guiTownCenterPanel ) {
            if ( idx == 7 ) {
                
            } else if ( idx == 6 ) {
                //OnGuiDrawPanel( &guiTownCenterUnlocksPanel, guiLayer, glm::vec2( -1, -1 ) );
                GUIGamePanelSmol smol =  GuiCreatePanelSmol( glm::vec2( 276, 42 ) );
                smol.leftImage = ResourceGetAndLoadSprite( "res/game/ents/ui/ui_icon_power.spr.json" );
                //smol.leftImage = ResourceGetAndLoadSprite( "res/game/ents/ui/ui_icon_money.spr.json" );
                OnGuiPanelDrawSmol( &smol, guiLayer, glm::vec2( -1, -1 ) );
            }
        }
    }

    void VisMap::OnGUIPanelClicked( DrawContext * guiLayer, i32 idx ) {
        if ( guiCurrentPanel == &guiTownCenterPanel ) {
            if ( idx == 7 ) {
                guiTownCenterPanel.activeIndex = idx;
                inputSprite = ResourceGetAndLoadSprite( "res/game/ents/structures/smol_reactor/blue/smol_reactor.spr.json" );
                inputMode = InputMode::PLACING_STRUCTURE;
                inputPlacingBuildingType = EntityType::STRUCTURE_SMOL_REACTOR;
            } else if ( idx == 6 ) {
                guiTownCenterPanel.activeIndex = idx;
                inputSprite = ResourceGetAndLoadSprite( "res/game/ents/structures/town_center/blue/town_center.spr.json" );
                inputMode = InputMode::PLACING_STRUCTURE;
                inputPlacingBuildingType = EntityType::STRUCTURE_CITY_CENTER;
            }
        }
    }
}

