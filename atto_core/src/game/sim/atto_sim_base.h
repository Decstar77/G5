#pragma once

#include "../../shared/atto_core.h"
#include "atto_sim_load_assets.h"

namespace atto {
    class RpcHolder;

    constexpr static i32 MAX_ENTITIES = 1024;
    constexpr static i32 MAX_PLAYERS = 4;

    constexpr i32 TURNS_PER_SECOND = 24;
    constexpr float SIM_DT_FLOAT = 1.0f / TURNS_PER_SECOND;
    constexpr fp TURNS_PER_SECOND_FP= Fp( TURNS_PER_SECOND );
    constexpr fp SIM_DT = Fp( 1.0f / TURNS_PER_SECOND );
    inline i32 SecondsToTurns( i32 s ) { return s * TURNS_PER_SECOND; }
    inline i32 SecondsToTurns( i32 s, i32 decisecond ) { return s * TURNS_PER_SECOND + decisecond * TURNS_PER_SECOND / 10; }
    inline i32 MinutesToTurns( i32 m ) { return m * 60 * TURNS_PER_SECOND; }
    inline i32 SecondsToTurns( fp s ) { return ToInt( FpRound( s * TURNS_PER_SECOND_FP ) ); }
    //inline static i32 MAX_MAP_SIZE = (i32)glm::sqrt( ToFloat( FP_MAX ) );// @NOTE: This comes to +5000
    inline static i32 MAX_MAP_SIZE = 3000;

    typedef TypeSafeNumber<i32, class PlayerNumberType>         PlayerNumber;
    typedef TypeSafeNumber<i32, class TeamNumberType>           TeamNumber;
    typedef TypeSafeNumber<i32, class SolaySystemNumberType>    SolarNumber;

    class SimMap;
    struct SimEntity;

    typedef ObjectHandle<SimEntity> EntityHandle;
    typedef FixedList<SimEntity *, MAX_ENTITIES>        EntList;
    typedef FixedList<const SimEntity *, MAX_ENTITIES>  ConstEntList;
    typedef FixedList<SimEntity, MAX_ENTITIES>          EntCacheList;
    typedef FixedList<EntityHandle, MAX_ENTITIES>       EntHandleList;
    typedef FixedObjectPool<SimEntity, MAX_ENTITIES>    EntPool;

    REFL_ENUM( EntityType,
              INVALID = 0,
              UNITS_BEGIN,
              UNIT_KLAED_WORKER,
              UNIT_KLAED_SCOUT,
              UNIT_KLAED_FIGHTER,
              UNIT_KLAED_BOMBER,
              UNIT_KLAED_TORPEDO,
              UNIT_KLAED_FRIGATE,
              UNIT_KLAED_BATTLE_CRUISER,
              UNIT_KLAED_DREADNOUGHT,

              UNIT_NAIRAN_WORKER,
              UNIT_NAIRAN_SCOUT,
              UNIT_NAIRAN_FIGHTER,

              UNIT_NAUTOLAN_WORKER,
              UNIT_NAUTOLAN_SCOUT,
              UNIT_NAUTOLAN_FIGHTER,

              UNITS_END,

              BULLETS_BEGIN,
              BULLET_KLARD_BULLET,
              PROJECTILE_NAIRAN_BOLT,
              PROJECTILE_NAIRAN_ROCKET,
              BULLET_MED,
              BULLETS_END,

              STAR,
              PLANET,

              BUILDING_BEGIN,
              BUILDING_STATION,
              BUILDING_TRADE,
              BUILDING_SOLAR_ARRAY,
              BUILDING_COMPUTE,
              BUILDING_END,

              TYPE_PROP
              );

    enum class MapActionType : u8 {
        NONE = 0,

        // These are all the actions that can be taken by the player and serialized across the network.
        PLAYER_SELECTION,
        SIM_ENTITY_UNIT_COMMAND_MOVE,
        SIM_ENTITY_UNIT_COMMAND_ATTACK,
        SIM_ENTITY_UNIT_COMMAND_CONSTRUCT_BUILDING,
        SIM_ENTITY_UNIT_COMMAND_CONSTRUCT_EXISTING_BUILDING,
        SIM_ENTITY_PLANET_COMMAND_PLACE_PLACEMENT,
        SIM_ENTITY_BUILDING_COMMAND_TRAIN_UNIT,
        SIM_ENTITY_BUILDING_COMMAND_CANCEL_UNIT,

        // These are all the actions that can be taken by the simulation, they are not serialized across the network. It's expected that the determinism of the simulation will be the same across all clients.
        SIM_ENTITY_SPAWN,
        SIM_ENTITY_DESTROY,

        SIM_ENTITY_APPLY_DAMAGE,
        SIM_ENTITY_APPLY_CONSTRUCTION,

        SIM_MAP_MONIES_GIVE_CREDITS,
        SIM_MAP_MONIES_GIVE_ENERGY,
        SIM_MAP_MONIES_GIVE_COMPUTE,

        TEST,

        COUNT
    };

    inline const char * MapActionTypeStrings[] = {
        "NONE",
        "PLAYER_SELECTION",
        "SIM_ENTITY_UNIT_COMMAND_MOVE",
        "SIM_ENTITY_UNIT_COMMAND_ATTACK",
        "SIM_ENTITY_UNIT_COMMAND_CONSTRUCT_BUILDING",
        "SIM_ENTITY_UNIT_COMMAND_CONSTRUCT_EXISTING_BUILDING",
        "SIM_ENTITY_PLANET_COMMAND_PLACE_PLACEMENT",
        "SIM_ENTITY_BUILDING_COMMAND_TRAIN_UNIT",
        "SIM_ENTITY_BUILDING_COMMAND_CANCEL_UNIT",
        "SIM_ENTITY_SPAWN",
        "SIM_ENTITY_DESTROY",
        "SIM_ENTITY_APPLY_DAMAGE",
        "SIM_ENTITY_APPLY_CONSTRUCTION",
        "SIM_MAP_MONIES_GIVE_CREDITS",
        "SIM_MAP_MONIES_GIVE_ENERGY",
        "SIM_MAP_MONIES_GIVE_COMPUTE",
        "TEST"
    };

    static_assert( (i32)ArrayCount( MapActionTypeStrings ) == (i32)MapActionType::COUNT, "You are missing an MapActionTypeString" );

    inline FixedList< RpcHolder *, 256 >         rpcTable = {};

    enum class EntitySelectionChange : u8 {
        SET,
        ADD,
        REMOVE
    };

    inline static const char * EntitySelectionChangeStrings[] = { 
        "SET",
        "ADD",
        "REMOVE"
    };

    class MapActionBuffer {
    public:
        void PlayerSelect( PlayerNumber playerNumber, const EntHandleList & entList, EntitySelectionChange change );
        void ConstructBuilding( PlayerNumber playerNumber, EntityType::_enumerated buildingType, fp2 pos );
        void TrainUnit( PlayerNumber playerNumber, EntityType::_enumerated unitType );

    public:
        FixedBinaryBlob<200>    data;

        template<typename... _types_>
        inline void AddAction( MapActionType type, _types_... args ) {
            static_assert( is_present<EntityType, _types_...>::value == false, "AddAction :: Must pass EntityType as i32" );
            static_assert( is_present<EntityType::_enumerated, _types_...>::value == false, "AddAction :: Must pass EntityType as i32" );
        #if ATTO_GAME_CHECK_RPC_FUNCTION_TYPES
            bool sameParms = rpcTable[ (i32)type ]->AreParamtersTheSame<void, std::add_pointer_t< _types_ >...>();
            AssertMsg( sameParms, "AddAction :: Adding an action with no corrasponding RPC function, most likey the parameters are not the same. " );
        #endif
            data.Write( &type );
            DoSimSerialize( args... );
        }

    private:
        template< typename _type_ >
        inline void DoSimSerialize( _type_ type ) {
            static_assert( std::is_pointer<_type_>::value == false, "AddAction :: Cannot take pointers" );
            data.Write( &type );
        }

        template< typename _type_, typename... _types_ >
        inline void DoSimSerialize( _type_ type, _types_... args ) {
            static_assert( std::is_pointer<_type_>::value == false, "AddAction :: Cannot take pointers" );
            data.Write( &type );
            DoSimSerialize( args... );
        }
    };

    struct MapTurn {
        i64                     checkSum;
        i32                     turnNumber;
        PlayerNumber            playerNumber;
        MapActionBuffer         playerActions;
        MapActionBuffer         aiActions;
    };
}


