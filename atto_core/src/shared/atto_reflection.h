#pragma once

#include "atto_containers.h"
#include "atto_math.h"
#include "atto_binary_file.h"

#include <json/json.hpp>

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
    nlohmann::json JSON_Write( fp v );
    nlohmann::json JSON_Write( fp2 v );

    void            JSON_Read( const nlohmann::json & j, u8 & o );
    void            JSON_Read( const nlohmann::json & j, bool & o );
    void            JSON_Read( const nlohmann::json & j, i32 & o );
    void            JSON_Read( const nlohmann::json & j, i64 & o );
    void            JSON_Read( const nlohmann::json & j, u32 & o );
    void            JSON_Read( const nlohmann::json & j, u64 & o );
    void            JSON_Read( const nlohmann::json & j, f32 & o );
    void            JSON_Read( const nlohmann::json & j, f64 & o );
    void            JSON_Read( const nlohmann::json & j, SmallString & o );
    void            JSON_Read( const nlohmann::json & j, LargeString & o );
    void            JSON_Read( const nlohmann::json & j, glm::vec2 & o );
    void            JSON_Read( const nlohmann::json & j, glm::vec3 & o );
    void            JSON_Read( const nlohmann::json & j, glm::vec4 & o );
    void            JSON_Read( const nlohmann::json & j, glm::mat2 & o );
    void            JSON_Read( const nlohmann::json & j, glm::mat3 & o );
    void            JSON_Read( const nlohmann::json & j, glm::mat4 & o );
    void            JSON_Read( const nlohmann::json & j, fp & v );
    void            JSON_Read( const nlohmann::json & j, fp2 & v );

    struct TypeDescriptor {
        i32         size;
        SmallString name;
        virtual ~TypeDescriptor() {}
        virtual nlohmann::json      JSON_Write( const void * obj ) = 0;
        virtual void                JSON_Read( const nlohmann::json & j, const void * obj ) = 0;
        virtual LargeString         ToString( const void * obj ) = 0;
        virtual void                Binary_Write( const void * obj, BinaryBlob & f ) { INVALID_CODE_PATH; }
        virtual void                Binary_Read( void * obj, BinaryBlob & f ) { INVALID_CODE_PATH;}
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

        virtual LargeString ToString( const void * obj ) override {
            INVALID_CODE_PATH;
            return {};
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

    template < typename _numberType_, typename _classType_ >
    struct TypeDescriptor_TypeSafeNumber : TypeDescriptor {
        virtual nlohmann::json JSON_Write( const void * obj ) override {
            INVALID_CODE_PATH;
            return {};
        }

        virtual void JSON_Read( const nlohmann::json & j, const void * obj ) override {
            INVALID_CODE_PATH;
        }

        virtual LargeString ToString( const void * obj ) override {
            TypeSafeNumber< _numberType_, _classType_ > * num = ( TypeSafeNumber< _numberType_, _classType_ > * )obj;
            TypeDescriptor * t = TypeResolver< _numberType_ >::get();
            return t->ToString( &num->value );
        }
    };

    template < typename _numberType_, typename _classType_ >
    class TypeResolver< TypeSafeNumber< _numberType_, _classType_ > > {
    public:
        static TypeDescriptor * get() {
            static TypeDescriptor_TypeSafeNumber<_numberType_, _classType_> typeDesc;
            return &typeDesc;
        }
    };

    template <typename _type_, i32 cap>
    struct TypeDescriptor_FixedList : TypeDescriptor {
        static_assert( std::is_trivial<_type_>::value, "Type must be trivial" );
        TypeDescriptor * itemType;

        TypeDescriptor_FixedList( _type_ * ) {
            this->size = sizeof( FixedList<_type_, cap> );
            this->name = SmallString::FromLiteral( "FixedList" );
            this->itemType = TypeResolver<_type_>::get();
        }

        virtual nlohmann::json JSON_Write( const void * obj ) override {
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
                _type_ * item = &list->AddEmpty();
                itemType->JSON_Read( jj, item );
            }
        }

        virtual LargeString ToString( const void * obj ) override {
            FixedList< _type_, cap > * list = ( FixedList< _type_, cap > * )obj;
            LargeString result = {};
            result.Add( "{" );
            for( i32 i = 0; i < list->GetCount(); i++ ) {
                result.Add( itemType->ToString( list->Get( i ) ) );
            }
            result.Add( "}" );
            return result;
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

    template <typename _type_>
    struct TypeDescriptor_GrowList : TypeDescriptor {
        static_assert( std::is_trivial<_type_>::value, "Type must be trivial" );
        TypeDescriptor * itemType;

        TypeDescriptor_GrowList( _type_ * ) {
            this->size = sizeof( GrowableList<_type_> );
            this->name = SmallString::FromLiteral( "GrowableList" );
            this->itemType = TypeResolver<_type_>::get();
        }

        virtual nlohmann::json JSON_Write( const void * obj ) override {
            const GrowableList< _type_ > * list = ( const GrowableList< _type_ > * )obj;
            nlohmann::json j = nlohmann::json::array();
            for( i32 i = 0; i < list->GetCount(); i++ ) {
                j.push_back( itemType->JSON_Write( list->Get( i ) ) );
            }
            return j;
        }

        virtual void JSON_Read( const nlohmann::json & j, const void * obj ) override {
            GrowableList< _type_ > * list = ( GrowableList< _type_ > * )obj;
            list->Clear( true );
            for( const nlohmann::json & jj : j ) {
                _type_ * item = &list->AddEmpty();
                itemType->JSON_Read( jj, item );
            }
        }

        virtual LargeString ToString( const void * obj ) override {
            GrowableList< _type_ > * list = ( GrowableList< _type_ > * )obj;
            LargeString result = {};
            result.Add( "{" );
            for( i32 i = 0; i < list->GetCount(); i++ ) {
                result.Add( itemType->ToString( list->Get( i ) ) );
            }
            result.Add( "}" );
            return result;
        }
    };


    template <typename T>
    class TypeResolver<GrowableList<T>> {
    public:
        static TypeDescriptor * get() {
            static TypeDescriptor_GrowList<T> typeDesc( ( T * )nullptr );
            return &typeDesc;
        }
    };

    template <typename _type_>
    struct TypeDescriptor_Span : TypeDescriptor {
        static_assert( std::is_trivial<_type_>::value, "Type must be trivial" );
        TypeDescriptor * itemType;

        TypeDescriptor_Span( _type_ * ) {
            this->size = sizeof( Span<_type_> );
            this->name = SmallString::FromLiteral( "Span" );
            this->itemType = TypeResolver<_type_>::get();
        }

        virtual nlohmann::json JSON_Write( const void * obj ) override {
            AssertMsg( false, "Spans cannot be JSON written" );
            nlohmann::json j;
            return j;
        }

        virtual void JSON_Read( const nlohmann::json & j, const void * obj ) override {
            AssertMsg( false, "Spans cannot be JSON read" );
        }

        virtual LargeString ToString( const void * obj ) override {
            AssertMsg( false, "NOT IMPL" );
            return {};
        }
    };

    template <typename T>
    class TypeResolver<Span<T>> {
    public:
        static TypeDescriptor * get() {
            static TypeDescriptor_Span<T> typeDesc( ( T * )nullptr );
            return &typeDesc;
        }
    };

    template<typename _type_ >
    struct TypeDescriptor_ObjectHandle : TypeDescriptor {
        virtual nlohmann::json JSON_Write( const void * obj ) override {
            INVALID_CODE_PATH;
            return {};
        }

        virtual void JSON_Read( const nlohmann::json & j, const void * obj ) override {
            INVALID_CODE_PATH;
        }

        virtual LargeString ToString( const void * obj ) override {
            ObjectHandle< _type_ > hndl = *( ObjectHandle< _type_ > * )obj;
            LargeString str = StringFormat::Large( "(idx=%d, gen%d)", hndl.idx, hndl.gen );
            return str;
        }
    };

    template <typename _type_>
    class TypeResolver< ObjectHandle< _type_ > > {
    public:
        static TypeDescriptor * get() {
            static TypeDescriptor_ObjectHandle< _type_ > typeDesc;
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

        virtual LargeString ToString( const void * obj ) override {
            INVALID_CODE_PATH;
            return {};
        }

        virtual void Binary_Write( const void * obj, BinaryBlob & f ) {
            INVALID_CODE_PATH;
        }

        virtual void Binary_Read( void * obj, BinaryBlob & f ) {
            INVALID_CODE_PATH;
        }
    };

    template< typename T, i32 cap >
    class TypeResolver< FixedObjectPool< T, cap > > {
    public:
        static TypeDescriptor * get() {
            static TypeDescriptor_FixedObjectPool< T, cap > typeDesc( ( T * )nullptr );
            return &typeDesc;
        }
    };
}

