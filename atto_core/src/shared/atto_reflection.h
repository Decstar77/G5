#pragma once

#include "atto_containers.h"
#include "atto_math.h"
#include "atto_binary_file.h"

#include <vector>
#include <iostream>
#include <string>
#include <cstddef>

#ifndef ATTO_SERVER

#include "json/json.hpp"
#include "imgui.h"

namespace atto {

#define AF_IGNORE

    nlohmann::json JSON_Write( bool v );
    nlohmann::json JSON_Write( i32 v );
    nlohmann::json JSON_Write( i64 v );
    nlohmann::json JSON_Write( u8 v );
    nlohmann::json JSON_Write( u32 v );
    nlohmann::json JSON_Write( u64 v );
    nlohmann::json JSON_Write( f32 v );
    nlohmann::json JSON_Write( f64 v );

    nlohmann::json JSON_Write( const SmallString & v );
    nlohmann::json JSON_Write( const LargeString & v );

    nlohmann::json JSON_Write( glm::vec2 v );
    nlohmann::json JSON_Write( glm::vec3 v );
    nlohmann::json JSON_Write( glm::vec4 v );
    nlohmann::json JSON_Write( glm::mat2 v );
    nlohmann::json JSON_Write( glm::mat3 v );
    nlohmann::json JSON_Write( glm::mat4 v );

    void JSON_Read( const nlohmann::json & j, u8 & o );
    void JSON_Read( const nlohmann::json & j, bool & o );
    void JSON_Read( const nlohmann::json & j, i32 & o );
    void JSON_Read( const nlohmann::json & j, i64 & o );
    void JSON_Read( const nlohmann::json & j, u32 & o );
    void JSON_Read( const nlohmann::json & j, u64 & o );
    void JSON_Read( const nlohmann::json & j, f32 & o );
    void JSON_Read( const nlohmann::json & j, f64 & o );
    void JSON_Read( const nlohmann::json & j, SmallString & o );
    void JSON_Read( const nlohmann::json & j, LargeString & o );
    void JSON_Read( const nlohmann::json & j, glm::vec2 & o );
    void JSON_Read( const nlohmann::json & j, glm::vec3 & o );
    void JSON_Read( const nlohmann::json & j, glm::vec4 & o );
    void JSON_Read( const nlohmann::json & j, glm::mat2 & o );
    void JSON_Read( const nlohmann::json & j, glm::mat3 & o );
    void JSON_Read( const nlohmann::json & j, glm::mat4 & o );

    struct TypeDescriptor {
        i32         size;
        SmallString name;
        virtual ~TypeDescriptor() {}
        virtual nlohmann::json      JSON_Write( const void * obj ) = 0;
        virtual void                JSON_Read( const nlohmann::json & j, const void * obj ) = 0;
        virtual void                Binary_Read( void * obj, BinaryBlob & f ) { f.Read( obj, size ); }
        virtual void                Binary_Write( const void * obj, BinaryBlob & f ) { f.Write( obj, size ); }
        virtual void                Imgui_Draw( const void * obj, const char * memberName ) = 0;
    };

    template <typename T>
    TypeDescriptor * GetPrimitiveDescriptor();

    struct DefaultResolver {
        template <typename T> static char func( decltype( &T::Reflection ) );
        template <typename T> static int func( ... );

        template <typename T>
        struct IsReflected {
            enum { value = ( sizeof( func<T>( nullptr ) ) == sizeof( char ) ) };
        };

        // This version is called if T has a static member named "Reflection":
        template <typename T, typename std::enable_if<IsReflected<T>::value, int>::type = 0>
        static TypeDescriptor * get() {
            return &T::Reflection;
        }

        // This version is called otherwise:
        template <typename T, typename std::enable_if<!IsReflected<T>::value, int>::type = 0>
        static TypeDescriptor * get() {
            return GetPrimitiveDescriptor<T>();
        }
    };

    template <typename T>
    struct TypeResolver {
        static TypeDescriptor * get() {
            return DefaultResolver::get<T>();
        }
    };

    struct TypeDescriptor_Struct : TypeDescriptor {
        struct Member {
            i32 offset;
            SmallString name;
            struct TypeDescriptor * type;
        };
        
        std::vector<Member> members;

        TypeDescriptor_Struct( void ( *init )( TypeDescriptor_Struct * ) ) {
            name = "";
            size = 0;
            init( this );
        }
        
        TypeDescriptor_Struct( const char * name, i32 size, const std::initializer_list<Member> & init ) {
            this->name = SmallString::FromLiteral( name );
            this->size = size;
            for( const Member & member : init ) {
                members.push_back( member );
            }
        }

        virtual nlohmann::json JSON_Write( const void * obj ) override {
            nlohmann::json j;
            for( const Member & member : members ) {
                j[ member.name.GetCStr() ] = member.type->JSON_Write( (char *)obj + member.offset );
            }
            return j;
        }

        virtual void JSON_Read( const nlohmann::json & j, const void * obj ) override {
            for( const Member & member : members ) {
                if( j.contains( member.name.GetCStr() ) == true ) {
                    const nlohmann::json & jj = j[ member.name.GetCStr() ];
                    member.type->JSON_Read( jj, (char *)obj + member.offset );
                }
            }
        }

        virtual void Imgui_Draw( const void * obj, const char * memberName ) override {
            nlohmann::json j;
            if ( ImGui::TreeNodeEx( memberName, ImGuiTreeNodeFlags_DefaultOpen ) ) {
                for( const Member & member : members ) {
                    member.type->Imgui_Draw( (char *)obj + member.offset, member.name.GetCStr() );
                }

                ImGui::TreePop();
            }
        }

        virtual void Binary_Read( void * obj, BinaryBlob & f ) override {
            for( const Member & member : members ) {
                member.type->Binary_Read( (char *)obj + member.offset, f );
            }
        }

        virtual void Binary_Write( const void * obj, BinaryBlob & f ) override {
            for( const Member & member : members ) {
                member.type->Binary_Write( (char *)obj + member.offset, f );
            }
        }
    };

#define REFLECT() \
    friend struct DefaultResolver; \
    static TypeDescriptor_Struct Reflection; \
    static void initReflection(TypeDescriptor_Struct*);

#define REFLECT_STRUCT_BEGIN(type) \
    TypeDescriptor_Struct type::Reflection{type::initReflection}; \
    void type::initReflection(TypeDescriptor_Struct* typeDesc) { \
        using T = type; \
        typeDesc->name = #type; \
        typeDesc->size = sizeof(T); \
        typeDesc->members = {

#define REFLECT_STRUCT_MEMBER(name) \
            { offsetof(T, name), SmallString::FromLiteral(#name), TypeResolver<decltype(T::name)>::get()},

#define REFLECT_STRUCT_END() \
        }; \
    }

    template <typename _type_, i32 cap>
    struct TypeDescriptor_FixedList : TypeDescriptor {
        TypeDescriptor *    itemType;
        
        TypeDescriptor_FixedList( _type_ * ) {
            this->size = sizeof( FixedList<_type_, cap> );
            this->name = SmallString::FromLiteral( "FixedList" );
            this->itemType = TypeResolver<_type_>::get();
        }

        virtual nlohmann::json TypeDescriptor_FixedList::JSON_Write( const void * obj ) override {
            const FixedList< _type_, cap > * list = ( const FixedList< _type_, cap > * )obj;
            nlohmann::json j = nlohmann::json::array();
            for( i32 i = 0; i < list->GetCount(); i++ ) {
                j.push_back( itemType->JSON_Write( list->Get( i ) ) );
            }
            return j;
        }

        virtual void JSON_Read( const nlohmann::json & j, const void * obj ) override {
            FixedList< _type_, cap > * list = ( FixedList< _type_, cap > * )obj;
            list->Clear( true );
            for( const nlohmann::json & jj : j ) {
                _type_ item = {};
                itemType->JSON_Read( jj, &item );
                list->Add( item );
            }
        }

        virtual void Imgui_Draw( const void * obj, const char * memberName ) override {
            FixedList< _type_, cap > * list = ( FixedList< _type_, cap > * )obj;
        #if 1
            if( ImGui::TreeNodeEx( memberName, ImGuiTreeNodeFlags_DefaultOpen ) ) {
                for( i32 i = 0; i < list->GetCount(); i++ ) {
                    ImGui::PushID( i );
                    if( ImGui::Button( "-" ) == true ) {
                        list->RemoveIndex( i );
                        ImGui::PopID();
                        break;
                    }
                    ImGui::PopID();
                    ImGui::SameLine();
                    SmallString n = StringFormat::Small( "Index:%d", i );
                    itemType->Imgui_Draw( list->Get( i ), n.GetCStr() );
                }
                if( list->IsFull() == false && ImGui::Button( "+" ) ) {
                    list->AddEmpty();
                }
                ImGui::TreePop();
            }
        #else 
            if( ImGui::BeginListBox( memberName ) ) {
                for( i32 i = 0; i < list->GetCount(); i++ ) {
                    SmallString n = StringFormat::Small( "Index:%d", i );
                    itemType->Imgui_Draw( list->Get( i ), n.GetCStr() );
                }
                ImGui::EndListBox();
            }
        #endif
        }

        virtual void Binary_Read( void * obj, BinaryBlob & f ) override {
            FixedList< _type_, cap > * list = ( FixedList< _type_, cap > * )obj;
            list->Clear( true );
            i32 count = 0;
            f.Read( &count );
            for( i32 i = 0; i < count; i++ ) {
                _type_ * t = &list->AddEmpty();
                itemType->Binary_Read( t, f );
            }
        }

        virtual void Binary_Write( const void * obj, BinaryBlob & f ) override {
            const FixedList< _type_, cap > * list = ( const FixedList< _type_, cap > * )obj;
            const i32 count = list->GetCount();
            f.Write( &count );
            for( i32 i = 0; i < count; i++ ) {
                itemType->Binary_Write( list->Get( i ), f );
            }
        }
    };

    template <typename T, i32 cap>
    class TypeResolver<FixedList<T, cap>> {
    public:
        static TypeDescriptor * get() {
            static TypeDescriptor_FixedList<T, cap> typeDesc( ( T * )nullptr );
            return &typeDesc;
        }
    };
}

#endif

 