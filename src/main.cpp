#include <clang/AST/AST.h>
#include <clang/AST/ASTContext.h>
#include <clang/AST/Decl.h>
#include <clang/AST/DeclCXX.h>
#include <clang/AST/PrettyPrinter.h>
#include <clang/Basic/Diagnostic.h>
#include <clang/Basic/FileManager.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Basic/TargetInfo.h>
#include <clang/Frontend/ASTConsumers.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Frontend/FrontendActions.h>
#include <clang/Frontend/Utils.h>
#include <clang/Tooling/Tooling.h>
#include <hjson.h>

#include <algorithm>
#include <iostream>
#include <variant>

using namespace clang;

// Создание и настройка компилятора
CompilerInstance* createCompilerInstance( )
{
	auto* ci = new CompilerInstance( );
	ci->createDiagnostics( );

	auto targetOptions    = std::make_shared< TargetOptions >( );
	targetOptions->Triple = "x86_64-pc-windows-msvc";
	ci->setTarget( TargetInfo::CreateTargetInfo( ci->getDiagnostics( ), targetOptions ) );

	ci->createFileManager( );
	ci->createSourceManager( ci->getFileManager( ) );
	ci->createPreprocessor( TU_Complete );
	ci->createASTContext( );

	return ci;
}

namespace common
{
	constexpr size_t constHash( const std::string_view input )
	{
		return input.empty( ) ? 0 : input[ 0 ] + 33 * constHash( input.substr( 1 ) );
	}
} // namespace common

class TypeBuilder
{
	ASTContext& context_;

	QualType						    QuTy_;
	std::variant< std::string, int32_t, int64_t, bool, int8_t > vvalue_;

public:
	explicit TypeBuilder( ASTContext& context ) : context_( context )
	{
	}

	[[nodiscard]] QualType build( const std::string_view typeName )
	{
		switch ( common::constHash( typeName ) ) {
			case common::constHash( "bool" ): return QuTy_ = context_.BoolTy;
			case common::constHash( "i8" ): return QuTy_ = context_.CharTy;
			case common::constHash( "u8" ): return QuTy_ = context_.UnsignedCharTy;
			case common::constHash( "i16" ): return QuTy_ = context_.ShortTy;
			case common::constHash( "u16" ): return QuTy_ = context_.UnsignedShortTy;
			case common::constHash( "i32" ): return QuTy_ = context_.IntTy;
			case common::constHash( "u32" ): return QuTy_ = context_.UnsignedIntTy;
			case common::constHash( "i64" ): return QuTy_ = context_.LongLongTy;
			case common::constHash( "u64" ): return QuTy_ = context_.UnsignedLongLongTy;
			case common::constHash( "f32" ): return QuTy_ = context_.FloatTy;
			case common::constHash( "f64" ): return QuTy_ = context_.DoubleTy;
			case common::constHash( "cstr" ): return QuTy_ = context_.getPointerType( context_.CharTy );

			default: throw std::logic_error( "Unknown type" );
		}
	}

	template< class T > QualType build( T& val )
	{
		switch ( typeid( T ).hash_code( ) ) {
			case typeid( bool ).hash_code( ): return QuTy_ = context_.BoolTy;
			case typeid( int8_t ).hash_code( ): return QuTy_ = context_.CharTy;
			case typeid( int16_t ).hash_code( ): return QuTy_ = context_.ShortTy;
			case typeid( int32_t ).hash_code( ): return QuTy_ = context_.IntTy;
			case typeid( int64_t ).hash_code( ): return QuTy_ = context_.LongLongTy;
			case typeid( std::string ).hash_code( ): return QuTy_ = context_.getPointerType( context_.CharTy );
			case typeid( uint8_t ).hash_code( ): return QuTy_ = context_.UnsignedCharTy;
			case typeid( uint16_t ).hash_code( ): return QuTy_ = context_.UnsignedShortTy;
			case typeid( uint32_t ).hash_code( ): return QuTy_ = context_.UnsignedIntTy;
			case typeid( uint64_t ).hash_code( ): return QuTy_ = context_.UnsignedLongLongTy;
			case typeid( float ).hash_code( ): return QuTy_ = context_.FloatTy;
			case typeid( double ).hash_code( ): return QuTy_ = context_.DoubleTy;

			default: throw std::logic_error( "Unknown type" );
		}
	}

	QualType build( const Hjson::Value& v )
	{
		if ( v.type( ) == Hjson::Type::Map ) {
			auto x = v[ "_type" ];
			if ( x.type( ) == Hjson::Type::String ) return build( std::string_view( x.to_string( ) ) );
		}

		switch ( v.type( ) ) {
			case Hjson::Type::Undefined: throw std::logic_error( "Hmmm страно что ты сюда передаешь того чего нет на самом делел :)?" );
			case Hjson::Type::Null: throw std::logic_error( "Null type not supported" );
			case Hjson::Type::Bool: return context_.BoolTy;
			case Hjson::Type::Double: return context_.DoubleTy;
			case Hjson::Type::Int64: return context_.LongLongTy;
			case Hjson::Type::String: return context_.getPointerType( context_.CharTy );
			case Hjson::Type::Vector: throw std::logic_error( "Vector type not supported, хуй знает что с этим делать :)" );
			case Hjson::Type::Map: throw std::logic_error( "Map type not supported, это должно уйти в namespace :)!" );
		}
	}

	template< class T > VarDecl* build_var( NamespaceDecl* DC, Hjson::Value& v, std::string v_name = "" )
	{
		QualType type;

		if ( v.type( ) == Hjson::Type::Map ) {
			{
				const auto t = v[ "_type" ];
				if ( t.type( ) == Hjson::Type::String ) type = build( std::string_view( t.to_string( ) ) );
			}
			{
				if ( v_name.empty( ) ) {
					const auto n = v[ "_name" ];
					switch ( n.type( ) ) {
						case Hjson::Type::String: v_name = n.to_string( ); ;
						case Hjson::Type::Undefined:
						case Hjson::Type::Null:
						case Hjson::Type::Bool:
						case Hjson::Type::Double:
						case Hjson::Type::Int64:
						case Hjson::Type::Vector:
						case Hjson::Type::Map:;
					}
				}
			}
			if ( !v[ "_value" ].defined( ) ) throw std::logic_error( "Value not defined" );
		}

		if ( v_name.empty( ) ) throw std::logic_error( "Name not defined" );

		auto var = VarDecl::Create( context_,
					    DC,
					    SourceLocation( ),
					    SourceLocation( ),
					    &context_.Idents.get( v_name ),
					    type,
					    nullptr,
					    SC_None );

		var->setConstexpr( true );

		Expr* vl = nullptr;

		switch ( v.type( ) ) {
			case Hjson::Type::Undefined: break; // throw
			case Hjson::Type::Null: break;	    // throw
			case Hjson::Type::Bool: vl = CXXBoolLiteralExpr::Create( context_, v.to_int64( ), context_.BoolTy, SourceLocation( ) ); break;
			case Hjson::Type::Double: break;
			case Hjson::Type::Int64: break;
			case Hjson::Type::String: break;
			case Hjson::Type::Vector: break;    // throw
			case Hjson::Type::Map: break;	    // throw
		}

		DC->addDecl( var );

		return var;
	}
};

int main( )
{
	CompilerInstance* ci	       = createCompilerInstance( );
	ASTContext&	  context      = ci->getASTContext( );
	auto		  global_scope = context.getTranslationUnitDecl( );

	// Создание пространства имен
	NamespaceDecl* namespaceDecl = NamespaceDecl::Create( context,
							      context.getTranslationUnitDecl( ),
							      false,
							      SourceLocation( ),
							      SourceLocation( ),
							      &context.Idents.get( "config" ),
							      nullptr,
							      true );

	global_scope->addDecl( namespaceDecl );

	// Создание переменной int myVar;
	const auto tp		 = context.getPointerType( context.getConstType( context.CharTy ) );
	QualType   constCharType = context.getConstType( context.CharTy );
	QualType   arrayType	 = context.getStringLiteralArrayType( constCharType, 13 );
	QualType   pcharType	 = context.getPointerType( context.CharTy );
	VarDecl*   varDecl	 = VarDecl::Create( context,
					    namespaceDecl,
					    SourceLocation( ),
					    SourceLocation( ),
					    &context.Idents.get( "myVar" ),
					    pcharType,
					    nullptr,
					    SC_None );

	varDecl->setConstexpr( true );

	// Создание значения по умолчанию
	StringLiteral* defaultVal =
			StringLiteral::Create( context, "Hello, World!", StringLiteral::StringKind::UTF8, false, arrayType, SourceLocation( ) );

	// Установка значения по умолчанию для переменной
	varDecl->setInit( defaultVal );

	// Добавление класса в пространство имен
	namespaceDecl->addDecl( varDecl );

	NamespaceDecl* namespaceProject = NamespaceDecl::Create( context,
								 namespaceDecl,
								 false,
								 SourceLocation( ),
								 SourceLocation( ),
								 &context.Idents.get( "project" ),
								 nullptr,
								 true );

	namespaceDecl->addDecl( namespaceProject );

	// Вывод сгенерированного кода
	LangOptions    langOpts;
	PrintingPolicy policy( langOpts );

	global_scope->print( llvm::outs( ), policy );

	llvm::outs( ) << "\n";

	delete ci;

	return 0;
}
