#pragma once

#include "atto_sim_var.h"
#include "atto_sim_actions.h"

namespace atto {
    struct SimEntity;
    struct VisEntity;
    class SpriteResource;

    constexpr i32 TILE_MAX_X_COUNT = 96;
    constexpr i32 TILE_MAX_Y_COUNT = 64;
    constexpr i32 TILE_SIZE = 8;

    enum TileFlags {
        TILE_FLAG_NONE = 0,
        TILE_FLAG_UNWALKABLE = SetABit(1),
    };

    struct Tile {
        i32 flatIndex;
        i32 xIndex;
        i32 yIndex;
        i32 spriteXIndex;
        i32 spriteYIndex;
        i32 flags;
        fp2         wsCenterFp;
        FpAxisBox   wsBoundsFp;
        glm::vec2   wsCenterFl;
        BoxBounds2D wsBoundsFl;
        SpriteResource * spriteResource;
    };

    struct TileInterval {
        i32 xIndex;
        i32 yIndex;
        i32 xCount;
        i32 yCount;
    };

    class TileMap {
    public:
        void PlaceTile( i32 xIndex, i32 yIndex, SpriteResource * sprite, i32 tileXIndex, i32 tileYIndex, i32 flags );
        void FillBorder( SpriteResource * sprite, i32 spriteX, i32 spriteY, i32 flags );
        void Fill( SpriteResource * sprite, i32 spriteX, i32 spriteY, i32 flags );
        void DebugDraw( class DrawContext * drawContext );

        void PosToTileIndex( fp2 p, i32 & x, i32 & y );
        fp2  PosToTileBL( fp2 p );
        void MarkTilesBL( i32 xIndex, i32 yIndex, i32 xCount, i32 yCount, i32 flags );
        bool ContainsFlag( i32 xIndex, i32 yIndex, i32 xCount, i32 yCount, i32 flag );

        TileInterval IntervalForCollider( FpCollider collider, fp2 pos );

        FixedList<Tile, TILE_MAX_X_COUNT * TILE_MAX_Y_COUNT> tiles = {};
    };

    enum class UnitState {
        IDLE = 0,
        MOVING = 1,
        ATTACKING = 2
    };

    struct Unit {
        UnitState   state;
        fp          speed;
        fp          range;
    };

    struct PlayerMonies {
        i32 power;
        i32 cash;
    };

    inline bool IsUnitType( EntityType t ) { return t == EntityType::UNIT_SCOUT; }
    inline bool IsStructureType( EntityType t ) { return t == EntityType::STRUCTURE_CITY_CENTER || t == EntityType::STRUCTURE_SMOL_REACTOR; }

    struct SimEntity {
        EntityHandle                handle;
        EntityType                  type;
        PlayerNumber                playerNumber;
        TeamNumber                  teamNumber;
        FixedList<PlayerNumber, 4>  selectedBy;

        fp2                         pos;
        fp2                         lastPos;
        f32                         posTime;

        bool                        movable;
        FpCollider                  collider;
        Unit                        unit;
        fp2                         dest;
        EntityHandle                target;
        VisEntity *                 vis;

        inline FpCollider   ColliderFpWorldSpace( fp2 p ) const { FpCollider c = collider; FpColliderTranslate( &c, p ); return c; }
        inline Collider2D   ColliderFlWorldSpace( glm::vec2 p ) const { Collider2D c = FpColliderToCollider2D( collider ); c.Translate( p ); return c; }
    };

    struct EntityFile {
        EntityType type;
        bool                        movable;
        FpCollider                  collider;
        Unit                        unit;
    };
    
    struct EntitySpawnCreateInfo {
        fp2 pos;
        EntityType type;
        PlayerNumber playerNumber;
        TeamNumber teamNumber;
    };
   
    struct MapTurn {
        i64 checkSum;
        i32 turnNumber;
        MapActionBuffer actions;
    };

    class SimMap {
    public:
        void        SimInitialize();
        bool        SimDoneTicks();
        void        SimNextTick( f32 dt );
        void        SimNextTurn( MapTurn * player1Turn, MapTurn * player2Turn, i32 tickCount );

        bool        SimCanPlaceStructure( EntityType type, fp2 pos );

    public:
        i32                                             simTickNumber = 0;
        i32                                             simTickStopNumber = 0;
        FixedList<PlayerMonies, 2>                      playerMonies = {};
        FixedObjectPool<SimEntity, MAX_ENTITY_COUNT>    entityPool = {};
        TileMap                                         tileMap = {};

    protected:
        virtual VisEntity *     VisMap_OnSpawnEntity( EntitySpawnCreateInfo createInfo ) { return nullptr; }

    private:
        void        ApplyActions( MapActionBuffer * actionBuffer );

        void        Action_RequestMove( PlayerNumber playerNumber, fp2 p );
        void        Action_RequestAttack( PlayerNumber playerNumber, EntityHandle handle );
        void        Action_RequestPlaceStructure( PlayerNumber playerNumber, i32 entityType, fp2 p );
        void        Action_RequestSelectEntities( PlayerNumber playerNumber, Span<EntityHandle> entities );

        SimEntity * Action_CommandSpawnEntity( EntitySpawnCreateInfo createInfo );
        SimEntity * Action_CommandSpawnEntity( EntityType type, fp2 pos, PlayerNumber playerNumber, TeamNumber teamNumber );
        void        Action_CommandMove( EntityHandle entityHandle, fp2 p1, fp2 p2, f32 t1, f32 t2 );

        f32                                         simTimeAccum = 0;
        FixedList<SimEntity *, MAX_ENTITY_COUNT>    actionActiveEntities = {};
        FixedList<SimEntity *, MAX_ENTITY_COUNT>    utilActiveEntities = {};
    };
}

