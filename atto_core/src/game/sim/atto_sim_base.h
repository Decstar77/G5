#pragma once

#include "../../shared/atto_core.h"

namespace atto {
    class RpcHolder;

    constexpr static i32 MAX_ENTITIES = 1024;
    constexpr static i32 MAX_PLAYERS = 4;

    constexpr i32 TURNS_PER_SECOND = 10;
    constexpr float SIM_DT_FLOAT = 1.0f / TURNS_PER_SECOND;
    constexpr fp SIM_DT = Fp( 1.0f / TURNS_PER_SECOND );
    inline i32 SecondsToTurns( i32 s ) { return s * TURNS_PER_SECOND; }
    
    //inline static i32 MAX_MAP_SIZE = (i32)glm::sqrt( ToFloat( FP_MAX ) );// @NOTE: This comes to +5000
    inline static i32 MAX_MAP_SIZE = 3000;

    typedef TypeSafeNumber<i32, class PlayerNumberType> PlayerNumber;
    typedef TypeSafeNumber<i32, class TeamNumberType>   TeamNumber;

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

        // These are all the actions that can be taken by the simulation, they are not serialized across the network. It's expected that the determinism of the simulation will be the same across all clients.
        SIM_ENTITY_SPAWN,
        SIM_ENTITY_DESTROY,

        SIM_ENTITY_APPLY_DAMAGE,
        SIM_ENTITY_APPLY_CONSTRUCTION,

        SIM_MAP_MONIES_GIVE_CREDITS,
        SIM_MAP_MONIES_GIVE_ENERGY,
        SIM_MAP_MONIES_GIVE_COMPUTE
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
        "SIM_ENTITY_SPAWN",
        "SIM_ENTITY_DESTROY",
        "SIM_ENTITY_APPLY_DAMAGE",
        "SIM_ENTITY_APPLY_CONSTRUCTION",
        "SIM_MAP_MONIES_GIVE_CREDITS",
        "SIM_MAP_MONIES_GIVE_ENERGY",
        "SIM_MAP_MONIES_GIVE_COMPUTE"
    };

    inline FixedList< RpcHolder *, 256 >         rpcTable = {};

    class MapActionBuffer {
    public:
        template<typename... _types_>
        inline void AddAction( MapActionType type, _types_... args ) {
            static_assert( is_present<EntityType, _types_...>::value == false, "AddAction :: Must pass EntityType as i32" );
            static_assert( is_present<EntityType::_enumerated, _types_...>::value == false, "AddAction :: Must pass EntityType as i32" );
            #if ATTO_GAME_CHECK_RPC_FUNCTION_TYPES
            bool sameParms = rpcTable[ (i32)type ]->AreParamtersTheSame<void, std::add_pointer_t< _types_ >...>();
            AssertMsg( sameParms, "AddAction :: Adding an action with no corrasponding RPC function, most likey the parameters are not the same. ");
            #endif
            data.Write( &type );
            DoSimSerialize( args... );
        }

    public:
        FixedBinaryBlob<256>    data;

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
        MapActionBuffer         actions;
    };


    //// @TODO: This is hardcoded to be 1 hours of game time for two players. We'll need a better system but for now this works.
    //struct MapReplay {
    //    static constexpr i64 Minutes = 120;
    //    static constexpr i64 TurnCap = ( i64 )TURNS_PER_SECOND * Minutes * 60;
    //    static constexpr i64 EstimatedSizeMB = ( ( ( sizeof( MapTurn ) * TurnCap ) / 1024 ) / 1024 );

    //    FixedList<MapTurn, TurnCap> turns;
    //};
}


