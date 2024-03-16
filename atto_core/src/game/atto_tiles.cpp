#pragma once

#include "atto_tiles.h"
#include "../shared/atto_core.h"

namespace atto {
    void TileMap::Initialize() {
        for( i32 y = 0; y < TILE_MAX_X_COUNT; y++ ) {
            for( i32 x = 0; x < TILE_MAX_Y_COUNT; x++ ) {
                PlaceTile( x, y, 0, 0, 0 );
            }
        }

        const i32 cap = TILE_MAX_X_COUNT * TILE_MAX_Y_COUNT;
        for( i32 i = 0; i < cap; i++ ) {
            tiles[ i ].active = false;
        }

       
    }

    i32 TileMap::GetFlatIndex( i32 x, i32 y ) {
        return y * tileXCount + x; // TILE_MAX_X_COUNT because this allows for resizing the map as each tile is in it's 'global' position
        //return y * TILE_MAX_X_COUNT + x; // TILE_MAX_X_COUNT because this allows for resizing the map as each tile is in it's 'global' position
    }

    TileMapTile * TileMap::GetTile( i32 x, i32 y ) {
        if( x >= 0 && x < tileXCount && y >= 0 && y < tileYCount ) {
            i32 flatIndex = GetFlatIndex( x, y );
            return &tiles[ flatIndex ];
        }
        return nullptr;
    }

    TileMapTile * TileMap::GetTile( glm::vec2 worldPos ) {
        i32 x = (i32)( worldPos.x / TILE_SIZE );
        i32 y = (i32)( worldPos.y / TILE_SIZE );
        return GetTile( x, y );
    }

    void TileMap::GetApron( i32 x, i32 y, FixedList<TileMapTile *, 9> & apron ) {
        for( i32 yIndex = -1; yIndex <= 1; yIndex++ ) {
            for( i32 xIndex = -1; xIndex <= 1; xIndex++ ) {
                i32 xCheck = x + xIndex;
                i32 yCheck = y + yIndex;
                if( xCheck >= 0 && xCheck < tileXCount && yCheck >= 0 && yCheck < tileYCount ) {
                    i32 flatIndex = GetFlatIndex( xCheck, yCheck );
                    TileMapTile * tile = &tiles[ flatIndex ];
                    apron.Add( tile );
                }
            }
        }
    }

    void TileMap::PlaceTile( i32 xIndex, i32 yIndex, i32 spriteX, i32 spriteY, i32 flags ) {
        TileMapTile tile = {};
        tile.active = true;
        tile.xIndex = xIndex;
        tile.yIndex = yIndex;
        tile.flatIndex = GetFlatIndex( xIndex, yIndex );
        tile.spriteTileIndexX = spriteX;
        tile.spriteTileIndexY = spriteY;
        tile.flags = flags;

        tile.center = glm::vec2( xIndex * TILE_SIZE + TILE_SIZE * 0.5f, yIndex * TILE_SIZE + TILE_SIZE * 0.5f );
        tile.wsBounds.min = tile.center + glm::vec2( -TILE_SIZE / 2.0f, -TILE_SIZE / 2.0f );
        tile.wsBounds.max = tile.center + glm::vec2( TILE_SIZE / 2.0f, TILE_SIZE / 2.0f );
        tiles[ tile.flatIndex ] = tile;
    }

    u32 MapFile::ResolveId( u32 gid, SmallString & source ) {
        const unsigned FLIPPED_HORIZONTALLY_FLAG = 0x80000000;
        const unsigned FLIPPED_VERTICALLY_FLAG = 0x40000000;
        const unsigned FLIPPED_DIAGONALLY_FLAG = 0x20000000;
        const unsigned ROTATED_HEXAGONAL_120_FLAG = 0x10000000;

        gid &= ~( FLIPPED_HORIZONTALLY_FLAG |
                  FLIPPED_VERTICALLY_FLAG |
                  FLIPPED_DIAGONALLY_FLAG |
                  ROTATED_HEXAGONAL_120_FLAG );

        const i32 idSetCount = tilesets.GetCount();
        for( i32 idSetIndex = idSetCount - 1; idSetIndex >= 0; idSetIndex-- ) {
            const MapFileIdSet & idSet = tilesets[ idSetIndex ];
            if( idSet.firstgid <= gid ) {
                source = idSet.source;
                return gid - idSet.firstgid;
            }
        }
        
        return 0;
    }
}

namespace atto {
    REFLECT_STRUCT_BEGIN( MapFileTileProperties )
        REFLECT_STRUCT_MEMBER( name )
        REFLECT_STRUCT_END()


        REFLECT_STRUCT_BEGIN( MapFileTile )
        REFLECT_STRUCT_MEMBER( id )
        REFLECT_STRUCT_MEMBER( gid )
        REFLECT_STRUCT_MEMBER( name )
        REFLECT_STRUCT_MEMBER( type )
        REFLECT_STRUCT_MEMBER( x )
        REFLECT_STRUCT_MEMBER( y )
        REFLECT_STRUCT_MEMBER( width )
        REFLECT_STRUCT_MEMBER( height )
        REFLECT_STRUCT_MEMBER( properties )
        REFLECT_STRUCT_END()

        REFLECT_STRUCT_BEGIN( MapFileTileSet )
        REFLECT_STRUCT_MEMBER( name )
        REFLECT_STRUCT_MEMBER( columns )
        REFLECT_STRUCT_MEMBER( tilewidth )
        REFLECT_STRUCT_MEMBER( tileheight )
        REFLECT_STRUCT_MEMBER( tiles )
        REFLECT_STRUCT_END()

        REFLECT_STRUCT_BEGIN( MapFileLayer )
        REFLECT_STRUCT_MEMBER( name )
        REFLECT_STRUCT_MEMBER( type )
        REFLECT_STRUCT_MEMBER( data )
        REFLECT_STRUCT_MEMBER( objects )
        REFLECT_STRUCT_END()

        REFLECT_STRUCT_BEGIN( MapFileIdSet )
        REFLECT_STRUCT_MEMBER( source )
        REFLECT_STRUCT_MEMBER( firstgid )
        REFLECT_STRUCT_END()

        REFLECT_STRUCT_BEGIN( MapFile )
        REFLECT_STRUCT_MEMBER( width )
        REFLECT_STRUCT_MEMBER( height )
        REFLECT_STRUCT_MEMBER( layers )
        REFLECT_STRUCT_MEMBER( tilesets )
        REFLECT_STRUCT_END()
}


