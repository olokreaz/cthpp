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
#include <clang/Frontend/ASTUnit.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Frontend/FrontendActions.h>
#include <clang/Frontend/Utils.h>
#include <clang/Tooling/Tooling.h>
#include <llvm/ADT/APFixedPoint.h>

#include <conjure_enum.hpp>

#include <algorithm>
#include <iostream>
#include <variant>

#include <libassert/assert.hpp>
#include <argh.h>
#include <git2.h>
#include <windows.h>

using namespace clang;

std::string getHashGitCommit( )
{
	static std::string hash;
	if ( !hash.empty( ) ) return hash;
	std::string c_path;
	c_path.resize( MAX_PATH );
	c_path.resize( GetCurrentDirectoryA( MAX_PATH, c_path.data( ) ) );

	git_libgit2_init( );

	git_repository* repo = nullptr;

	if ( const int er = git_repository_open( &repo, c_path.data( ) ) ) {
		git_libgit2_shutdown( );
		const git_error* err = giterr_last( );
		llvm::errs( ) << "Error: " << er << " " << ( err && err->message ? err->message : "Uknown error" ) << "\n";
		return { };
	}

	git_oid oid;
	git_reference_name_to_id( &oid, repo, "HEAD" );

	git_commit* cmt = nullptr;
	git_commit_lookup( &cmt, repo, &oid );
	hash.resize( 8 );
	git_oid_tostr( hash.data( ), 8, &oid );

	git_commit_free( cmt );
	git_repository_free( repo );
	git_libgit2_shutdown( );

	llvm::outs( ) << c_path << ": last git commit Hash: " << hash << "\n";

	return hash;
}

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
	constexpr size_t const_hash( const std::string_view input )
	{
		return input.empty( ) ? 0 : input[ 0 ] + 33 * const_hash( input.substr( 1 ) );
	}
} // namespace common

class TypeBuilder
{
public:
	enum class Types : uint8_t {
		none = 0,

		boolean = 1,

		i8,
		u8,

		i16,
		u16,

		i32,
		u32,

		i64,
		u64,

		f32,
		f64,

		string,
	};
	using e_type = FIX8::conjure_enum< Types >;

private:
	ASTContext& ctx_;

public:
	explicit TypeBuilder( ASTContext& context ) : ctx_( context )
	{
	}

	[[nodiscard]] QualType GetType( const std::string_view typeName )
	{
		auto tp = e_type::unscoped_string_to_enum( typeName ).value_or( Types::none );
		return GetType( tp );
	}
	[[nodiscard]] QualType GetType( const Types tp )
	{
		switch ( tp ) {
			case Types::boolean: return ctx_.BoolTy;
			case Types::i8: return ctx_.CharTy;
			case Types::u8: return ctx_.UnsignedCharTy;
			case Types::i16: return ctx_.ShortTy;
			case Types::u16: return ctx_.UnsignedShortTy;
			case Types::i32: return ctx_.IntTy;
			case Types::u32: return ctx_.UnsignedIntTy;
			case Types::i64: return ctx_.LongLongTy;
			case Types::u64: return ctx_.UnsignedLongLongTy;
			case Types::f32: return ctx_.FloatTy;
			case Types::f64: return ctx_.DoubleTy;
			case Types::string: return ctx_.getPointerType( ctx_.CharTy );

			case Types::none:
			default: throw std::logic_error( "Unknown type" );
		}
	}

	Expr* BuildInitStatement( const Types tp, std::string_view init_state )
	{

		switch ( tp ) { // TODO: Autocorrect Redix (use ctre)
			case Types::boolean: {

				return CXXBoolLiteralExpr::Create( ctx_,
								   !( init_state == "false" || init_state == "0" ),
								   GetType( "bool" ),
								   SourceLocation( ) );
			}
			case Types::i8: {

				return clang::IntegerLiteral::Create( ctx_, llvm::APInt( 8, init_state, 10 ), GetType( "i8" ), SourceLocation( ) );
			}
			case Types::i16: {
				return clang::IntegerLiteral::Create( ctx_, llvm::APInt( 16, init_state, 10 ), GetType( "i16" ), SourceLocation( ) );
			}
			case Types::i32: {

				return clang::IntegerLiteral::Create( ctx_, llvm::APInt( 32, init_state, 10 ), GetType( "i32" ), SourceLocation( ) );
			}
			case Types::i64: {
				return clang::IntegerLiteral::Create( ctx_, llvm::APInt( 64, init_state, 10 ), GetType( "i64" ), SourceLocation( ) );
			}
			case Types::f32: {
				float xfl;
				std::from_chars( &( *init_state.begin( ) ), &( *init_state.end( ) ), xfl );
				return clang::FloatingLiteral::Create( ctx_, llvm::APFloat( xfl ), false, GetType( "f32" ), SourceLocation( ) );
			}
			case Types::f64: {
				double xw;
				std::from_chars( &( *init_state.begin( ) ), &( *init_state.end( ) ), xw );
				return clang::FloatingLiteral::Create( ctx_, llvm::APFloat( xw ), false, GetType( "f64" ), SourceLocation( ) );
			}
			case Types::string: {
				return clang::StringLiteral::Create( ctx_,
								     init_state,
								     StringLiteral::Unevaluated,
								     false,
								     ctx_.getStringLiteralArrayType( ctx_.CharTy, init_state.size( ) ),
								     SourceLocation( ) );
			}
			default: {
				// Обработка случая по умолчанию, если нужно
				return nullptr;
			}
		}
	}
};

NamespaceDecl* CreateNamespace( llvm::StringRef name, ASTContext& ctx, DeclContext* dcctx )
{
	NamespaceDecl* ns_global =
			NamespaceDecl::Create( ctx, dcctx, false, SourceLocation( ), SourceLocation( ), &ctx.Idents.get( name ), nullptr, true );

	return ns_global;
}

void createVar( ASTContext& ctx, NamespaceDecl* ns, const llvm::StringRef name, const QualType type, Expr* init )
{
	VarDecl* varDecl = VarDecl::Create( ctx, ns, SourceLocation( ), SourceLocation( ), &ctx.Idents.get( name ), type, nullptr, SC_None );

	varDecl->setConstexpr( true );
	varDecl->setInit( init );

	ns->addDecl( varDecl );
}

#include <jsoncons/json.hpp>

using json = jsoncons::json;

namespace ConfParser
{
	struct Project {
		std::string name;
		std::string desc;
		std::string output_name;
		std::string version;
		std::string git_hash;
	};

	/*
	 {
		project: {
			..
		},
	 }
	 */
	Project parse( const json& j )
	{
		Project project;

		const auto& j_project = j[ "project" ];

		project.name	    = j_project[ "name" ].as< std::string >( );
		project.desc	    = j_project[ "desc" ].as< std::string >( );
		project.output_name = j_project[ "output_name" ].as< std::string >( );
		project.version	    = j_project[ "version" ].as< std::string >( );
		project.git_hash    = getHashGitCommit( );

		return project;
	}

	void appendProjectNamespace( ASTContext& ctx, const Project& p, NamespaceDecl* ns )
	{
		auto str_qt = TypeBuilder( ctx ).GetType( "string" );
		createVar( ctx, ns, "name", str_qt, TypeBuilder( ctx ).BuildInitStatement( TypeBuilder::Types::string, p.name ) );
		createVar( ctx, ns, "description", str_qt, TypeBuilder( ctx ).BuildInitStatement( TypeBuilder::Types::string, p.desc ) );
		createVar( ctx, ns, "commit_hash", str_qt, TypeBuilder( ctx ).BuildInitStatement( TypeBuilder::Types::string, p.git_hash ) );
		createVar( ctx, ns, "version", str_qt, TypeBuilder( ctx ).BuildInitStatement( TypeBuilder::Types::string, p.version ) );
		//createVar( ctx, ns, "name", str_qt, TypeBuilder( ctx ).BuildInitStatement( TypeBuilder::Types::string, p.name ) );
	}

} // namespace ConfParser

int main( int argc, char** argv )
{
	CompilerInstance*    ci		  = createCompilerInstance( );
	ASTContext&	     context	  = ci->getASTContext( );
	TranslationUnitDecl* global_scope = context.getTranslationUnitDecl( );

	const argh::parser opt( argc, argv );

	// Создание пространства имен
	NamespaceDecl* ns_global_config = CreateNamespace( opt( { "-ns", "--global-namespace" }, "config" ).view( ), context, global_scope );

	DEBUG_ASSERT( ns_global_config );

	NamespaceDecl* namespaceProject = CreateNamespace( "project", context, ns_global_config );

	const ConfParser::Project proj = ConfParser::parse( json::parse_file( opt( { "-f", "--file" }, "config.json" ).view( ) ) );

	ConfParser::appendProjectNamespace( context, proj, namespaceProject );

	ns_global_config->addDecl( namespaceProject );
	global_scope->addDecl( ns_global_config );

	// Вывод сгенерированного кода
	LangOptions    langOpts;
	PrintingPolicy policy( langOpts );

	llvm::outs( ) << "#pragma once\n";
	llvm::outs( ) << "#define _Bool bool\n\n";

	global_scope->print( llvm::outs( ), policy );

	delete ci;

	return 0;
} // namespace ConfHJsonint main(intargc,char**argv)
