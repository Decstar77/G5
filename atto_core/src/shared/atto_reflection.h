#pragma once

#include "atto_containers.h"
#include "atto_random.h"
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
        virtual void                Binary_Write( const void * obj, BinaryBlob & f ) { f.Write( obj, size ); }
        virtual void                Binary_Read( void * obj, BinaryBlob & f ) { f.Read( obj, size ); }
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
            if( ImGui::TreeNodeEx( memberName, ImGuiTreeNodeFlags_DefaultOpen ) ) {
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
        static_assert( std::is_trivial<_type_>::value, "Type must be trivial" );
        TypeDescriptor * itemType;

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
            _type_ * item = new _type_();
            for( const nlohmann::json & jj : j ) {
                ZeroStructPtr( item );
                itemType->JSON_Read( jj, item );
                list->Add_MemCpyPtr( item );
            }
            delete item;
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

    template <typename _type_, i32 cap>
    struct TypeDescriptor_FixedObjectPool : TypeDescriptor {
        TypeDescriptor * itemType;

        TypeDescriptor_FixedObjectPool( _type_ * ) {
            this->size = sizeof( FixedObjectPool<_type_, cap> );
            this->name = SmallString::FromLiteral( "FixedObjectPool" );
            this->itemType = TypeResolver<_type_>::get();
        }

        typedef FixedList< _type_, cap > PoolList;

        virtual nlohmann::json JSON_Write( const void * obj ) {
            std::unique_ptr< PoolList > activeItems = std::make_unique<PoolList>();
            FixedObjectPool<_type_, cap> * pool = ( FixedObjectPool<_type_, cap> * )obj;
            pool->GatherActiveObjs_MemCopy( activeItems.get() );
            TypeDescriptor * poolListType = TypeResolver<PoolList>::get();
            return poolListType->JSON_Write( activeItems.get() );
        };

        virtual void JSON_Read( const nlohmann::json & j, const void * obj ) {
            std::unique_ptr< PoolList > activeItems = std::make_unique<PoolList>();
            TypeDescriptor * poolListType = TypeResolver<PoolList>::get();
            poolListType->JSON_Read( j, activeItems.get() );
            FixedObjectPool<_type_, cap> * pool = ( FixedObjectPool<_type_, cap> * )obj;
            const i32 count = activeItems->GetCount();
            for( i32 i = 0; i < count; i++ ) {
                ObjectHandle<_type_> handle = {};
                _type_ * newT = pool->Add( handle );
                _type_ * t = activeItems->Get( i );
                memcpy( newT, t, sizeof( _type_ ) );
                newT->handle = handle;
            }
        };

        virtual void Binary_Write( const void * obj, BinaryBlob & f ) {
            INVALID_CODE_PATH;
            f.Write( obj, size );
        }

        virtual void Binary_Read( void * obj, BinaryBlob & f ) {
            INVALID_CODE_PATH;
            f.Read( obj, size );
        }

        virtual void Imgui_Draw( const void * obj, const char * memberName ) {

        };
    };

    template< typename T, i32 cap >
    class TypeResolver< FixedObjectPool< T, cap > > {
    public:
        static TypeDescriptor * get() {
            static TypeDescriptor_FixedObjectPool< T, cap > typeDesc( ( T * )nullptr );
            return &typeDesc;
        }
    };

    template<typename _type_>
    inline static _type_ ReflEnumFromString( const char * str ) {
        _type_ type = _type_::Make( (_type_::_enumerated)( 0 ) );
        auto values = _type_::Values();
        auto names = _type_::Names();
        auto count = _type_::_count;
        for( size_t i = 0; i < count; i++ ) {
            if( strcmp( str, names[ i ] ) == 0 ) {
                type = (_type_::_enumerated)values[ i ];
            }
        }
        return type;
    }

#define MAP(macro, ...) \
    IDENTITY( \
        APPLY(CHOOSE_MAP_START, COUNT(__VA_ARGS__)) \
            (macro, __VA_ARGS__))

#define CHOOSE_MAP_START(count) MAP ## count

#define APPLY(macro, ...) IDENTITY(macro(__VA_ARGS__))

// Needed to expand __VA_ARGS__ "eagerly" on the MSVC preprocessor.
#define IDENTITY(x) x

#define MAP1(m, x)      m(x)
#define MAP2(m, x, ...) m(x) IDENTITY(MAP1(m, __VA_ARGS__))
#define MAP3(m, x, ...) m(x) IDENTITY(MAP2(m, __VA_ARGS__))
#define MAP4(m, x, ...) m(x) IDENTITY(MAP3(m, __VA_ARGS__))
#define MAP5(m, x, ...) m(x) IDENTITY(MAP4(m, __VA_ARGS__))
#define MAP6(m, x, ...) m(x) IDENTITY(MAP5(m, __VA_ARGS__))
#define MAP7(m, x, ...) m(x) IDENTITY(MAP6(m, __VA_ARGS__))
#define MAP8(m, x, ...) m(x) IDENTITY(MAP7(m, __VA_ARGS__))
#define MAP9(m, x, ...) m(x) IDENTITY(MAP8(m, __VA_ARGS__))
#define MAP10(m, x, ...) m(x) IDENTITY(MAP9(m, __VA_ARGS__))
#define MAP11(m, x, ...) m(x) IDENTITY(MAP10(m, __VA_ARGS__))
#define MAP12(m, x, ...) m(x) IDENTITY(MAP11(m, __VA_ARGS__))
#define MAP13(m, x, ...) m(x) IDENTITY(MAP12(m, __VA_ARGS__))
#define MAP14(m, x, ...) m(x) IDENTITY(MAP13(m, __VA_ARGS__))
#define MAP15(m, x, ...) m(x) IDENTITY(MAP14(m, __VA_ARGS__))
#define MAP16(m, x, ...) m(x) IDENTITY(MAP15(m, __VA_ARGS__))
#define MAP17(m, x, ...) m(x) IDENTITY(MAP16(m, __VA_ARGS__))
#define MAP18(m, x, ...) m(x) IDENTITY(MAP17(m, __VA_ARGS__))
#define MAP19(m, x, ...) m(x) IDENTITY(MAP18(m, __VA_ARGS__))
#define MAP20(m, x, ...) m(x) IDENTITY(MAP19(m, __VA_ARGS__))
#define MAP21(m, x, ...) m(x) IDENTITY(MAP20(m, __VA_ARGS__))
#define MAP22(m, x, ...) m(x) IDENTITY(MAP21(m, __VA_ARGS__))
#define MAP23(m, x, ...) m(x) IDENTITY(MAP22(m, __VA_ARGS__))
#define MAP24(m, x, ...) m(x) IDENTITY(MAP23(m, __VA_ARGS__))
#define MAP25(m, x, ...) m(x) IDENTITY(MAP24(m, __VA_ARGS__))
#define MAP26(m, x, ...) m(x) IDENTITY(MAP25(m, __VA_ARGS__))
#define MAP27(m, x, ...) m(x) IDENTITY(MAP26(m, __VA_ARGS__))
#define MAP28(m, x, ...) m(x) IDENTITY(MAP27(m, __VA_ARGS__))
#define MAP29(m, x, ...) m(x) IDENTITY(MAP28(m, __VA_ARGS__))
#define MAP30(m, x, ...) m(x) IDENTITY(MAP29(m, __VA_ARGS__))
#define MAP31(m, x, ...) m(x) IDENTITY(MAP30(m, __VA_ARGS__))
#define MAP32(m, x, ...) m(x) IDENTITY(MAP31(m, __VA_ARGS__))
#define MAP33(m, x, ...) m(x) IDENTITY(MAP32(m, __VA_ARGS__))

#define EVALUATE_COUNT(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, count, ...) count

#define COUNT(...) \
    IDENTITY(EVALUATE_COUNT(__VA_ARGS__, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1))


    struct ignore_assign {
        ignore_assign( int value ) : _value( value ) {}
        operator int() const { return _value; }

        const ignore_assign & operator =( int dummy ) { return *this; }

        int _value;
    };

#define IGNORE_ASSIGN_SINGLE(expression) (ignore_assign)expression,
#define IGNORE_ASSIGN(...) IDENTITY(MAP(IGNORE_ASSIGN_SINGLE, __VA_ARGS__))

#define STRINGIZE_SINGLE(expression) #expression,
#define STRINGIZE(...) IDENTITY(MAP(STRINGIZE_SINGLE, __VA_ARGS__))


#define REFL_ENUM(EnumName, ...)                                                                            \
struct EnumName {                                                                                           \
    enum _enumerated { __VA_ARGS__ };                                                                       \
                                                                                                            \
    _enumerated     _value;                                                                                 \
                                                                                                            \
    operator _enumerated() const { return _value; }                                                         \
    inline void operator=( _enumerated other ) { _value = other; }                                          \
                                                                                                            \
    inline const char* ToString() const                                                                     \
    {                                                                                                       \
        for (size_t index = 0; index < _count; ++index) {                                                   \
            if (Values()[index] == _value)                                                                  \
                return Names()[index];                                                                      \
        }                                                                                                   \
                                                                                                            \
        return NULL;                                                                                        \
    }                                                                                                       \
                                                                                                            \
    inline static const size_t _count = IDENTITY(COUNT(__VA_ARGS__));                                       \
                                                                                                            \
    inline static EnumName Make(_enumerated e){ return { e }; }                                             \
                                                                                                            \
    inline static const int* Values()                                                                       \
    {                                                                                                       \
        static const int values[] =                                                                         \
            { IDENTITY(IGNORE_ASSIGN(__VA_ARGS__)) };                                                       \
        return values;                                                                                      \
    }                                                                                                       \
                                                                                                            \
    inline static const char* const* Names()                                                                \
    {                                                                                                       \
        static const char* const    raw_names[] =                                                           \
            { IDENTITY(STRINGIZE(__VA_ARGS__)) };                                                           \
                                                                                                            \
        static char*                processed_names[_count];                                                \
        static bool                 initialized = false;                                                    \
                                                                                                            \
        if (!initialized) {                                                                                 \
            for (size_t index = 0; index < _count; ++index) {                                               \
                size_t length =                                                                             \
                    std::strcspn(raw_names[index], " =\t\n\r");                                             \
                                                                                                            \
                processed_names[index] = new char[length + 1];                                              \
                                                                                                            \
                std::strncpy(                                                                               \
                    processed_names[index], raw_names[index], length);                                      \
                processed_names[index][length] = '\0';                                                      \
            }                                                                                               \
        }                                                                                                   \
                                                                                                            \
        return processed_names;                                                                             \
    }                                                                                                       \
};                                                                                                          \
                                                                                                            \
    struct TypeDescriptor_##EnumName : TypeDescriptor {                                                     \
                                                                                                            \
      TypeDescriptor_##EnumName() {                                                                         \
          this->size = sizeof( EnumName );                                                                  \
          this->name = SmallString::FromLiteral( #EnumName );                                               \
      }                                                                                                     \
                                                                                                            \
      virtual nlohmann::json JSON_Write( const void * obj ) {                                               \
          EnumName en = *(EnumName *)obj;                                                                   \
          nlohmann::json j;                                                                                 \
          j[ name.GetCStr() ] = en.ToString();                                                              \
          return j;                                                                                         \
      }                                                                                                     \
                                                                                                            \
      virtual void JSON_Read( const nlohmann::json & j, const void * obj ) {                                \
            EnumName * en = (EnumName *)obj;                                                                \
            auto names = EnumName::Names();                                                                 \
            for (size_t index = 0; index < EnumName::_count; ++index) {                                     \
                if ( names[ index ] == j[ name.GetCStr() ]) {                                               \
                    *en = EnumName::Make( (EnumName::_enumerated)EnumName::Values()[index] );               \
                    return;                                                                                 \
                }                                                                                           \
            }                                                                                               \
        }                                                                                                   \
                                                                                                            \
      virtual void Imgui_Draw( const void * obj, const char * memberName ) {                                \
            ImGui::Text( "%s: %s", memberName, ((EnumName *)obj)->ToString() );                             \
      };                                                                                                    \
    };                                                                                                      \
                                                                                                            \
    template <>                                                                                             \
    inline TypeDescriptor * GetPrimitiveDescriptor<EnumName>() {                                            \
        static TypeDescriptor_##EnumName typeDesc;                                                          \
        return &typeDesc;                                                                                   \
    }
}

#endif                                                                                                      
