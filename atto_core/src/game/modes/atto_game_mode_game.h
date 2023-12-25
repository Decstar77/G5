#pragma once

#include "../atto_game.h"

namespace atto {
    constexpr static int    MAX_ENTITIES = 1024;

    class Map;
    struct Entity;
    enum EntityType;
    struct EntitySpawnInfo;
    
    typedef ObjectHandle<Entity> EntityHandle;
    typedef FixedList<Entity *, MAX_ENTITIES> EntList;

    typedef Entity * ( *EntityTypeFunc_Spawn ) ( Core * core, Map * map, EntitySpawnInfo & spawnInfo );
    typedef void     ( *EntityTypeFunc_Update )( Core * core, Map * map, Entity * ent, f32 dt );

    struct EntityTypeFuncInfo {
        EntityTypeFunc_Spawn spawnFunc;
        EntityTypeFunc_Update updateFunc;
    };

    struct EntitySpawnInfo {
        EntityType          type;
        glm::vec3           pos;
        glm::vec3           ori;
        EntityTypeFuncInfo  funcs;
    };

    enum EntityType {
        ENTITY_TYPE_INVALID = 0,
        ENTITY_TYPE_PLAYER = 1,
        ENTITY_TYPE_DRONE_01 = 2,
    };

    Entity * EntityTypeFunc_Spawn_Drone01( Core * core, Map * map, EntitySpawnInfo & spawnInfo );
    void     EntityTypeFunc_Update_Drone01( Core * core, Map * map, Entity * ent, f32 dt );

    // TODO: Remove std::vector and make fixed lists use initializer lists. Also I'm not sure if this should be global or apart of the map...
    inline std::vector<EntityTypeFuncInfo> ENTITY_TYPE_FUNCS = {
        { nullptr, nullptr },
        { nullptr, nullptr },
        { EntityTypeFunc_Spawn_Drone01, EntityTypeFunc_Update_Drone01 },
    };


    struct EntCamera {
        f32 yaw;
        f32 pitch;
        f32 movementSpeed;
        f32 mouseSensitivity;
        f32 yfov;
        f32 zNear;
        f32 zFar;
        glm::vec3 front;
        glm::vec3 up;
        glm::vec3 right;
        bool noclip;

        glm::mat3 GetOrientation() const;
        static EntCamera CreateDefault();
    };

    struct Entity {
        EntityHandle        handle;
        EntityType          type;
        EntityTypeFuncInfo  funcs;

        bool            sleeping;

        glm::vec3       pos;
        glm::mat3       ori;

        glm::vec3       vel;

        EntCamera           camera;

        f32                 fireRate;
        f32                 fireRateAccumulator;
        i32                 fireDamage;
        glm::vec3           localStartGunPos;
        glm::vec3           localGunPos;

        i32                 maxHealth;
        i32                 currentHealth;

        // Make these flags
        bool hasCollision;
        bool isCollisionStatic;
        bool isSelectable;

        Collider    selectionCollider;
        Collider    collisionCollider;

        bool selected;

        i32 teamNumber;

        bool                    wantsDraw;
        StaticMeshResource *    drawMesh;
        TextureResource *       drawTexture;

        glm::mat4   CameraGetViewMatrix() const;
        glm::mat4   Player_ComputeGunTransformMatrix() const;
    };

    struct MapTriangle {
        glm::vec3   p1;
        glm::vec3   p2;
        glm::vec3   p3;
        glm::vec2   uv1;
        glm::vec2   uv2;
        glm::vec2   uv3;
        glm::vec3   normal;

        inline void         ComputeNormal() { normal = glm::normalize( glm::cross( p2 - p1, p3 - p1 ) ); }
        inline void         InvertNormal() { normal = -normal; Swap( p1, p3 ); Swap( uv1, uv3 ); }
        inline glm::vec3    GetCenter() const { return ( p1 + p2 + p3 ) / 3.0f; }
    };

    struct MapBlock {
        bool filled;
        i32 flatIndex;
        i32 xIndex;
        i32 yIndex;
        glm::vec2 bottomLeftWS;
        glm::vec2 topRightWS;
    };

    struct MapFileBlock {
        i32 xIndex;
        i32 yIndex;

        REFLECT();
    };

    struct MapFileEntity {
        i32 type;
        glm::vec3 pos;
        glm::mat3 ori;

        REFLECT();
    };

    struct MapFile {
        i32 version;
        i32 mapWidth;
        i32 mapHeight;
        glm::vec3 playerStartPos;
        FixedList<MapFileBlock, 1024>   blocks;
        FixedList<MapFileEntity, 1024>  entities;

        REFLECT();
    };

    class Map {
    public:
        inline static f32 BlockDim = 4.0f;
        i32                             mapWidth;
        i32                             mapHeight;

        glm::vec3                       playerStartPos;

        FixedList<MapBlock, 1024>       blocks;
        FixedList<MapTriangle, 1024>    triangles;

        FixedObjectPool< Entity, MAX_ENTITIES > entityPool;
        f32                                     dtAccumulator = 0.0f;

        Entity * localPlayer = nullptr;

    public:
        // @NOTE: "Map Live" functions 
        void                            Start( Core * core );
        void                            UpdateAndRender( Core * core, f32 dt, UpdateAndRenderFlags flags );
        Entity *                        SpawnEntity( EntityType type );
        Entity *                        SpawnPlayer( glm::vec3 pos );
        void                            EntityUpdatePlayer( Core * core, Entity * ent );

        // @NOTE: "Map Build" functions 
        bool                            Edit_AddBlock( i32 x, i32 y );
        bool                            Edit_RemoveBlock( i32 x, i32 y );
        void                            Edit_AddFloor( glm::vec2 p1, glm::vec2 p2, i32 level, bool invertNormal );
        void                            Edit_AddWall( glm::vec2 p1, glm::vec2 p2, bool invertNormal );
        bool                            Edit_LoadFromMapFile( MapFile * mapFile );
        bool                            Edit_SaveToMapFile( MapFile * mapFile );
        void                            Edit_Bake();

        void                            DEBUG_SaveToFile( Core * core, const char * path );
        void                            DEBUG_LoadFromFile( Core * core, const char * path );

    public:
        TextureResource *               grid_Dark1 = nullptr;
        TextureResource *               grid_Dark8 = nullptr;
        TextureResource *               tex_PolygonScifi_01_C = nullptr;
        StaticMeshResource *            mesh_Wep_Pistol_Bot = nullptr;
        StaticMeshResource *            mesh_Enemy_Drone_Quad_01 = nullptr;
        AudioResource *                 snd_Gun_Pistol_Shot_01 = nullptr;

    };

    class GameModeGame : public GameMode {
    public:
        virtual GameModeType            GetGameModeType() override;
        virtual bool                    IsInitialized() override;
        virtual void                    Initialize( Core * core ) override;
        virtual void                    UpdateAndRender( Core * core, f32 dt, UpdateAndRenderFlags flags ) override;
        virtual void                    Shutdown( Core * core ) override;

    public:
        Map                                     map;

    };
}
