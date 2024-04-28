#include "../shared/atto_defines.h"
#include "../shared/atto_containers.h"

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <filesystem>

namespace atto {
    void * PlatformAllocateMemory( u64 size ) {
        return malloc( size );
    }

    void PlatformFreeMemory( void * buf ) {
        free( buf );
    }

    void PlatformReadEntireTextFile( const char * path, char * text, i32 maxLen ) {
          std::ifstream file( path );

        if( file.is_open() ) {
            file.seekg( 0, std::ios::end );
            std::streampos fileSize = file.tellg();
            file.seekg( 0, std::ios::beg );

            if( static_cast<i32>( fileSize ) > maxLen ) {
                std::cout << "DEBUG_ReadTextFile :: File size is too big. Truncating to " << maxLen << " bytes." << std::endl;
            }

            maxLen = Min( maxLen, static_cast<i32>( fileSize ) );

            file.read( text, maxLen );

            text[ maxLen ] = '\0';

            file.close();
        }
    }

    void PlatformWriteEntireTextFile( const char * path, const char * data ) {
        std::ofstream file( path );
        if( file.is_open() ) {
            file << data;
            file.close();
        }
    }

    void PlatformReadEntireBinaryFile( const char * path, char * data, i32 maxLen ) {
        std::ifstream file( path, std::ios::in | std::ios::binary );
        if( file.is_open() ) {
            file.read( data, maxLen );
            file.close();
        }
    }

    void PlatformWriteEntireBinaryFile( const char * path, const char * data, i32 size ) {
        std::ofstream file( path, std::ios::out | std::ios::binary );
        if( file.is_open() ) {
            file.write( data, size );
            file.close();
        }
    }

    i64 PlatformGetFileSize( const char * path ) {
        std::ifstream file( path );

        if( file.is_open() ) {
            file.seekg( 0, std::ios::end );
            std::streampos fileSize = file.tellg();
            file.close();
            return static_cast<i64>( fileSize );
        }
        else {
        }

        return -1;
    }

    static void FindAllFiles( const std::filesystem::path& folderPath, const char * filter, GrowableList<LargeString>& fileList ) {
        for (const auto& entry : std::filesystem::directory_iterator(folderPath)) {
            if (std::filesystem::is_regular_file(entry.path())) {
                std::string filePath = entry.path().string();
                std::string filename = entry.path().filename().string();
                size_t dotPos = filename.find_first_of( '.' );
                if( dotPos != std::string::npos ) {
                    std::string extension = filename.substr( dotPos + 1 );
                    if( extension == filter ) {
                        LargeString r = LargeString::FromLiteral( filePath.c_str() );
                        r.BackSlashesToSlashes();
                        fileList.Add( r );
                    }
                }
            } else if (std::filesystem::is_directory(entry.path())) {
                FindAllFiles(entry.path(), filter, fileList);
            }
        }
    }

    GrowableList<LargeString> PlatformGetAllFilesInFolderAndSubFolders( const char * folderPath, const char * filter ) {
        GrowableList<LargeString> fileList = {};
        FindAllFiles( folderPath, filter, fileList );
        return fileList;
    }
}
