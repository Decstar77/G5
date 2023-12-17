#pragma once

#include "../atto_game.h"

namespace atto {
    enum EntityType {
        ENTITY_TYPE_INVALID = 0,
        ENTITY_TYPE_PLAYER = 1,
    };

    constexpr static int    MAX_ENTITIES = 1024;

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
    };

    struct Entity;
    typedef ObjectHandle<Entity> EntityHandle;
    typedef FixedList<Entity *, MAX_ENTITIES> EntList;
    struct Entity {
        EntityHandle    handle;
        EntityType      type;
        glm::vec3       pos;
        glm::mat3       ori;

        glm::vec3       vel;

        EntCamera           camera;

        f32                 fireRate;
        f32                 fireRateAccumulator;
        i32                 fireDamage;

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

        glm::mat4   CameraGetViewMatrix() const;
    };

    struct MapTriangle {
        glm::vec3   p1;
        glm::vec3   p2;
        glm::vec3   p3;
        glm::vec3   normal;

        inline void         ComputeNormal() { normal = glm::normalize( glm::cross( p2 - p1, p3 - p1 ) ); }
        inline glm::vec3    GetCenter() const { return ( p1 + p2 + p3 ) / 3.0f; }
    };

    struct Map {
        FixedList<MapTriangle, 1024> triangles;
    };

    class GameModeGame : public GameMode {
    public:
        GameModeType GetGameModeType() override;
        void Init( Core * core ) override;
        void UpdateAndRender( Core * core, f32 dt ) override;
        void Shutdown( Core * core ) override;

        Entity * SpawnEntity( EntityType type );
        Entity * SpawnPlayer( glm::vec3 pos );

        void     EntityUpdatePlayer( Core * core, Entity * ent );
    public:

        Map                                     map;
        FixedObjectPool< Entity, MAX_ENTITIES > entityPool;
        f32                                     dtAccumulator = 0.0f;

        Entity *                                localPlayer = nullptr;
        

    };
}
