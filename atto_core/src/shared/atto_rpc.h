#pragma once

#include "atto_containers.h"
#include "atto_binary_file.h"
#include "atto_reflection.h"

namespace atto {
    template< typename _type_ >
    struct is_span {
        static const bool value = false;
    };

    template< typename _type_ >
    struct is_span< Span< _type_ > > {
        static const bool value = true;
        static const size_t sizeOfElement = sizeof( _type_ );
        typedef _type_ innner_type;
    };

    template< typename _type_ >
    struct is_fixed_list {
        static const bool value = false;
    };

    template< typename _type_, i32 cap >
    struct is_fixed_list< FixedList< _type_, cap > > {
        static const bool value = true;
        static const size_t sizeOfElement = sizeof( _type_ );
    };

    template< typename _type_ >
    struct is_grow_list {
        static const bool value = false;
    };

    template< typename _type_ >
    struct is_grow_list< GrowableList< _type_ > > {
        static const bool value = true;
        static const size_t sizeOfElement = sizeof( _type_ );
        typedef _type_ innner_type;
    };

    template<typename What, typename ... Args>
    struct is_present {
        static constexpr bool value{ ( std::is_same_v<What, Args> || ... ) };
    };

    template<typename _ret_, typename ... _args_>
    class RpcBase;

    class RpcHolder {
    public:
        virtual void Call( char * data ) = 0;
        virtual i32 GetLastCallSize() const = 0;
        virtual i32 GetParameterCount() const = 0;
        virtual LargeString Log( char * data ) = 0;

        template<typename _ret_, typename ... _args_>
        inline bool AreParamtersTheSame() {
            const i32 count = RpcBase<_ret_, _args_... >::instances.GetCount();
            for( i32 i = 0; i < count; i++ ) {
                if( RpcBase<_ret_, _args_... >::instances[ i ] == this ) {
                    return true;
                }
            }

            return false;
        }
    };

    template<typename _ret_, typename ... _args_>
    class RpcBase : public RpcHolder {
    public:
        inline static FixedList< RpcBase<_ret_, _args_...> *, 32 > instances;

        RpcBase() {
            instances.Add( this );
        }

        virtual ~RpcBase() {
            instances.RemoveValue( this );
        }

        template<typename _type_>
        inline _type_ Convert( size_t argIndex, const std::array< size_t, sizeof...( _args_ ) > & sizes, char * data ) {
            COMPILE_ASSERT( std::is_pointer_v<_type_> == false, "RpcBase :: Argument can't be a pointer" );

            size_t dataOffset = 0;
            for( size_t i = 0; i < argIndex; i++ ) {
                dataOffset += sizes[ i ];
            }

            _type_ result = *(_type_ *)( data + dataOffset );

            if constexpr( is_span< _type_ >::value ) {
                using InnerType = typename is_span<_type_>::innner_type;
                result.data = (InnerType *)( data + dataOffset + sizeof( i32 ) );
            }

            return result;
        }

        template<size_t... S>
        inline _ret_ CallImpl( std::index_sequence<S...>, const std::array< size_t, sizeof...( _args_ ) > & sizes, char * data ) {
            return DoCall( Convert<_args_>( S, sizes, data )... );
        }

        template<typename _type_>
        inline size_t GetSize( typename std::enable_if< !is_span< _type_ >::value >::type * = 0 ) {
            return sizeof( _type_ );
        }

        template<typename _type_>
        inline size_t GetSize( typename std::enable_if< is_span< _type_ >::value >::type * = 0 ) {
            return is_span< _type_ >::sizeOfElement;
        }

        template<typename _type_>
        inline bool MarkSpan( typename std::enable_if< !is_span< _type_ >::value >::type * = 0 ) {
            return false;
        }

        template<typename _type_>
        inline bool MarkSpan( typename std::enable_if< is_span< _type_ >::value >::type * = 0 ) {
            return true;
        }

        inline size_t GetSizeFromSpan( size_t index, const std::array< size_t, sizeof...( _args_ ) > & sizes, char * data ) {
            size_t dataOffset = 0;
            for( size_t i = 0; i < index; i++ ) {
                dataOffset += sizes[ i ];
            }

            const i32 count = *(i32 *)( data + dataOffset );
            const i32 size = (i32)sizes[ index ] * count;
            const i32 total = size + (i32)sizeof( i32 );// size of the list + size of the count
            return (size_t)total;
        }

        virtual _ret_ Call( char * data ) override {
            auto seq = std::index_sequence_for< _args_... >{};
            auto sizes = std::array< size_t, sizeof...( _args_ ) > { GetSize< _args_ >()... };
            auto spanMarks = std::array< bool, sizeof...( _args_ ) > { MarkSpan< _args_ >()... };

            lastCallSize = 0;
            for( size_t i = 0; i < sizeof...( _args_ ); i++ ) {
                if( spanMarks[ i ] == true ) {
                    sizes[ i ] = GetSizeFromSpan( i, sizes, data );
                }
                
                lastCallSize += sizes[ i ];
            }

            return CallImpl( seq, sizes, data );
        };

        template<typename _type_>
        inline LargeString ConvertString( size_t argIndex, const std::array< size_t, sizeof...( _args_ ) > & sizes, char * data ) {
            COMPILE_ASSERT( std::is_pointer_v<_type_> == false, "RpcBase :: Argument can't be a pointer" );
            COMPILE_ASSERT( is_fixed_list<_type_>::value == false, "RpcBase cannot take Raw Lists, please use a Span" );
            COMPILE_ASSERT( is_grow_list<_type_>::value == false, "RpcBase cannot take Raw Lists, please use a Span" );

            size_t dataOffset = 0;
            for( size_t i = 0; i < argIndex; i++ ) {
                dataOffset += sizes[ i ];
            }

            _type_ * objPtr = (_type_ *)( data + dataOffset );
            TypeDescriptor * leType = TypeResolver< _type_ >::get();
            LargeString res = leType->ToString( objPtr );

            return res;
        }

        void CatStrings() {
        }

        template<typename _type_, typename... Args>
        void CatStrings( _type_ f, Args... args ) {
            logString.Add( f );
            logString.Add( ", " );
            CatStrings( args... );
        }

        template<size_t... S>
        inline void LogImpl( std::index_sequence<S...>, const std::array< size_t, sizeof...( _args_ ) > & sizes, char * data ) {
            CatStrings( ConvertString<_args_>( S, sizes, data )... );
        }

        virtual LargeString Log( char * data ) override {
            auto seq = std::index_sequence_for< _args_... >{};
            auto sizes = std::array< size_t, sizeof...( _args_ ) > { GetSize< _args_ >()... };
            auto spanMarks = std::array< bool, sizeof...( _args_ ) > { MarkSpan< _args_ >()... };

            lastCallSize = 0;
            for( size_t i = 0; i < sizeof...( _args_ ); i++ ) {
                if( spanMarks[ i ] == true ) {
                    sizes[ i ] = GetSizeFromSpan( i, sizes, data );
                }
                
                lastCallSize += sizes[ i ];
            }

            logString.Clear();
            LogImpl( seq, sizes, data );
            return logString;
        }

        virtual i32 GetParameterCount() const {
            return (i32)sizeof...( _args_ );
        }

        virtual i32 GetLastCallSize() const override { return (i32)lastCallSize; }
        virtual _ret_ DoCall( _args_ ... args ) = 0;

    private:
        LargeString logString;
        size_t lastCallSize = 0;
    };

    template<typename _ret_, typename ... _args_>
    class RpcFunction : public RpcBase<_ret_, _args_...> {
    public:
        typedef _ret_( *func )( _args_ ... );
        func f;

        RpcFunction( func f ) { this->f = f; }

        virtual _ret_ DoCall( _args_... args ) override {
            return f( std::forward<_args_>( args )... );
        }
    };

    template<typename ClassType, typename ReturnType, typename... Args>
    using MemberFunctionPtr = ReturnType( ClassType:: * )( Args... );

    template<typename _class_, typename _ret_, typename ... _args_>
    class RpcMemberFunction : public RpcBase<_ret_, _args_...> {
    public:
        _class_ * objectPtr;
        MemberFunctionPtr<_class_, _ret_, _args_...> functionPtr;

        RpcMemberFunction( _class_ * obj, MemberFunctionPtr<_class_, _ret_, _args_...> func )
            : objectPtr( obj ), functionPtr( func ) {}

        virtual _ret_ DoCall( _args_... args ) override {
            return ( objectPtr->*functionPtr )( std::forward<_args_>( args )... );
        }
    };

    inline FixedList< RpcHolder *, 256 > GlobalRpcTable = {};

    class RpcBuffer {
    public:
        FixedBinaryBlob<200>    data;

    public:
        void Combine( RpcBuffer & other ) {
            data.Write( other.data.GetBuffer(), other.data.GetSize() );
        }

        template<typename... _types_>
        inline void AddAction( i32 funcId, _types_... args ) {
            #if ATTO_GAME_CHECK_RPC_FUNCTION_TYPES
            bool sameParms = GlobalRpcTable[ (i32)funcId ]->AreParamtersTheSame<void, _types_...>();
            AssertMsg( sameParms, "AddAction :: Adding an action with no corrasponding RPC function, most likey the parameters are not the same. " );
            #endif

            data.Write( &funcId );
            DoSerialize( args... );
        }

    private:
        template< typename _type_ >
        inline void DoSerialize( _type_ type ) {
            COMPILE_ASSERT( std::is_pointer<_type_>::value == false, "AddAction :: Cannot take pointers" );
            COMPILE_ASSERT( is_fixed_list<_type_>::value == false, "RpcBuffer cannot take Raw Lists, please cast/pass in a Span : list.GetSpan()" );
            COMPILE_ASSERT( is_grow_list<_type_>::value == false, "RpcBuffer cannot take Raw Lists, please cast/pass in a Span : list.GetSpan()" );
            data.Write( &type );
        }
        
        template< typename _type_, typename... _types_ >
        inline void DoSerialize( _type_ type, _types_... args ) {
            COMPILE_ASSERT( std::is_pointer<_type_>::value == false, "AddAction :: Cannot take pointers" );
            COMPILE_ASSERT( is_fixed_list<_type_>::value == false, "RpcBuffer cannot take Raw Lists, please cast/pass in a Span : list.GetSpan()" );
            COMPILE_ASSERT( is_grow_list<_type_>::value == false, "RpcBuffer cannot take Raw Lists, please cast/pass in a Span : list.GetSpan()" );
            data.Write( &type );
            DoSerialize( args... );
        }
    };

}
