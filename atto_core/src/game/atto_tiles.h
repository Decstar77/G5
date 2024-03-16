#pragma once

#include "../shared/atto_math.h"

namespace atto {
    class Core;
    class SpriteResource;

    enum TileFlags {
        TILE_FLAG_NO_WALK = SetABit( 1 )
    };

    struct TileMapTile {
        bool                active;
        i32                 xIndex;
        i32                 yIndex;
        i32                 flatIndex;
        glm::vec2           center;
        BoxBounds2D         wsBounds;
        i32                 spriteTileIndexX;
        i32                 spriteTileIndexY;
        i32                 flags;
    };

    static constexpr f32                TILE_SIZE = 32.0f;
    static constexpr i32                TILE_MAX_X_COUNT = 100;
    static constexpr i32                TILE_MAX_Y_COUNT = 100;
    typedef FixedList<TileMapTile, TILE_MAX_X_COUNT * TILE_MAX_Y_COUNT> TileMapList;

    class TileMap {
    public:
        i32                                 tileXCount;
        i32                                 tileYCount;
        SpriteResource *                    spriteResource;
        TileMapList                         tiles;

    public:
        void                                Initialize();
        i32                                 GetFlatIndex( i32 x, i32 y );
        TileMapTile *                       GetTile( i32 x, i32 y );
        TileMapTile *                       GetTile( glm::vec2 worldPos );
        void                                GetApron( i32 x, i32 y, FixedList<TileMapTile *, 9> & apron );
        void                                PlaceTile( i32 xIndex, i32 yIndex, i32 spriteX, i32 spriteY, i32 flags );
    };

    struct MapFileTileProperties {
        SmallString name;
        REFLECT();
    };

    struct MapFileTile {
        u32         id;
        u32         gid;
        SmallString name;
        SmallString type;
        f32         x;
        f32         y;
        f32         width;
        f32         height;
        FixedList< MapFileTileProperties, 32 >  properties;

        REFLECT();
    };

    struct MapFileTileSet {
        SmallString                                  name;
        i32                                          columns;
        i32                                          tilewidth;
        i32                                          tileheight;
        FixedList<MapFileTile, 256>                  tiles;

        REFLECT();
    };

    struct MapFileLayer {
        SmallString                                 name;
        SmallString                                 type;
        FixedList<i32, 1024>                        data;
        FixedList<MapFileTile, 1024>                objects;

        REFLECT();
    };

    struct MapFileIdSet {
        SmallString source;
        u32 firstgid;
        
        REFLECT();
    };

    struct MapFile {
        i32                                         width;
        i32                                         height;
        FixedList< MapFileLayer, 8 >                layers;
        FixedList< MapFileIdSet, 8 >                tilesets;

        u32                                         ResolveId( u32 gid, SmallString & source );

        REFLECT();
    };

}
