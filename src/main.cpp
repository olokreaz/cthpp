//    Copyright 2024 olokreZ ( Долгополов Василий Васильевич | Dolgopolov Vasily Vasilievich )
//
//    Licensed under the Apache License, Version 2.0 (the "License");
//    you may not use this file except in compliance with the License.
//    You may obtain a copy of the License at
//
//        http://www.apache.org/licenses/LICENSE-2.0
//
//    Unless required by applicable law or agreed to in writing, software
//    distributed under the License is distributed on an "AS IS" BASIS,
//    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//    See the License for the specific language governing permissions and
//    limitations under the License.

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
#include <llvm/Support/Host.h>

#include <conjure_enum.hpp>

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <variant>

#include <git2.h>

#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonpath/json_query.hpp>

#include <srilakshmikanthanp/libfiglet.hpp>

#include <llvm/Support/Regex.h>

#define VERSION_PACK( MAJOR, MINOR, PATCH ) ( ( ( MAJOR ) << 16 ) | ( ( MINOR ) << 8 ) | ( PATCH ) )
#define VERSION_MAJOR( VERSION )	    ( ( ( VERSION ) >> 16 ) & 0xFF )
#define VERSION_MINOR( VERSION )	    ( ( ( VERSION ) >> 8 ) & 0xFF )
#define VERSION_PATCH( VERSION )	    ( ( VERSION ) & 0xFF )

using namespace clang;

#include "./program_options.hpp"

std::string getHashGitCommit( const llvm::StringRef path = "" )
{
	if ( opt::NoGit ) return "";

	static std::string hash;
	if ( !hash.empty( ) ) return hash;

	if ( path.empty( ) ) return "";

	git_libgit2_init( );

	git_repository* repo = nullptr;

	if ( const int er = git_repository_open( &repo, path.data( ) ); er ) {
		git_libgit2_shutdown( );
		const git_error* err = giterr_last( );
		llvm::errs( ) << "git error " << er << " " << ( err && err->message ? err->message : "Uknown error" ) << "\n";
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

	return hash;
}

// Создание и настройка компилятора
CompilerInstance* createCompilerInstance( )
{
	auto* ci = new CompilerInstance( );
	ci->createDiagnostics( );

	auto targetOptions = std::make_shared< TargetOptions >( );

	targetOptions->Triple = llvm::sys::getDefaultTargetTriple( );
	targetOptions->CPU    = llvm::sys::getHostCPUName( ).str( );

	ci->setTarget( TargetInfo::CreateTargetInfo( ci->getDiagnostics( ), targetOptions ) );

	ci->createFileManager( );
	ci->createSourceManager( ci->getFileManager( ) );
	ci->createPreprocessor( TU_Complete );
	ci->createASTContext( );

	return ci;
}

namespace common {
	constexpr size_t const_hash( const std::string_view input )
	{
		return input.empty( ) ? 0 : input[ 0 ] + 33 * const_hash( input.substr( 1 ) );
	}
}    // namespace common

// void se_translator( unsigned int code, EXCEPTION_POINTERS* pExp )
// {
// 	std::string exceptionMessage = "Caught SEH exception: ";
//
// 	// Извлечение кода исключения
// 	exceptionMessage += "Code: 0x" + std::to_string( pExp->ExceptionRecord->ExceptionCode ) + ", ";
//
// 	// Адрес, по которому произошло исключение
// 	exceptionMessage += "Address: " + std::to_string( ( uintptr_t )pExp->ExceptionRecord->ExceptionAddress ) + ", ";
//
// 	// Если это нарушение доступа, выводим дополнительные сведения
// 	if ( pExp->ExceptionRecord->ExceptionCode == EXCEPTION_ACCESS_VIOLATION ) {
// 		exceptionMessage += "Access Type: " + std::to_string( pExp->ExceptionRecord->ExceptionInformation[ 0 ] ) + ", ";
// 		exceptionMessage += "Access Address: " + std::to_string( ( uintptr_t )pExp->ExceptionRecord->ExceptionInformation[ 1 ] );
// 	}
//
// 	throw std::runtime_error( exceptionMessage );
// }

class TypeBuilder
{
public:
	enum class Types : uint8_t
	{
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
			case Types::i8	   : return ctx_.CharTy;
			case Types::u8	   : return ctx_.UnsignedCharTy;
			case Types::i16	   : return ctx_.ShortTy;
			case Types::u16	   : return ctx_.UnsignedShortTy;

			case Types::i32	   : return ctx_.IntTy;
			case Types::u32	   : return ctx_.UnsignedIntTy;

			case Types::i64	   : return common::const_hash( opt::TargetArch ) == common::const_hash( "x64" ) ? ctx_.LongLongTy : ctx_.IntTy;
			case Types::u64:
				return common::const_hash( opt::TargetArch ) == common::const_hash( "x64" ) ? ctx_.UnsignedLongLongTy
													    : ctx_.UnsignedIntTy;

			case Types::f32	  : return ctx_.FloatTy;

			case Types::f64	  : return common::const_hash( opt::TargetArch ) == common::const_hash( "x64" ) ? ctx_.DoubleTy : ctx_.FloatTy;

			case Types::string: return ctx_.getPointerType( ctx_.CharTy );

			case Types::none  :
			default		  : throw std::logic_error( "[builder] Unknown type" );
		}
	}

	Expr* BuildInitStatement( const Types tp, std::string_view init_state )
	{

		switch ( tp ) {
			case Types::boolean: {

				return CXXBoolLiteralExpr::Create( ctx_,
								   !( init_state == "false" || init_state == "0" ),
								   GetType( tp ),
								   SourceLocation( ) );
			}
			case Types::u8:
			case Types::i8: {
				return clang::IntegerLiteral::Create( ctx_, llvm::APInt( 8, init_state, 10 ), GetType( tp ), SourceLocation( ) );
			}
			case Types::u16:
			case Types::i16: {
				return clang::IntegerLiteral::Create( ctx_, llvm::APInt( 16, init_state, 10 ), GetType( tp ), SourceLocation( ) );
			}
			case Types::u32:
			case Types::i32: {

				return clang::IntegerLiteral::Create( ctx_, llvm::APInt( 32, init_state, 10 ), GetType( tp ), SourceLocation( ) );
			}
			case Types::u64:
			case Types::i64: {
				return clang::IntegerLiteral::Create(
						ctx_,
						llvm::APInt( common::const_hash( opt::TargetArch ) == common::const_hash( "x64" ) ? 64 : 32,
							     init_state,
							     10 ),
						GetType( tp ),
						SourceLocation( ) );
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
	NamespaceDecl* ns_global
			= NamespaceDecl::Create( ctx, dcctx, false, SourceLocation( ), SourceLocation( ), &ctx.Idents.get( name ), nullptr, true );

	return ns_global;
}

void createVar( ASTContext& ctx, NamespaceDecl* ns, const llvm::StringRef name, const QualType type, Expr* init )
{
	VarDecl* varDecl = VarDecl::Create( ctx, ns, SourceLocation( ), SourceLocation( ), &ctx.Idents.get( name ), type, nullptr, SC_None );

	varDecl->setConstexpr( true );
	varDecl->setInit( init );

	ns->addDecl( varDecl );
}

using json = jsoncons::json;

namespace ConfParser {
	struct Project
	{
		std::string name;
		std::string desc;
		std::string git_hash;
		bool	    has_uncommited_changes{ false };

		uint32_t version{ VERSION_PACK( 1, 0, 0 ) };	// MAJOR.MINOR.PATCH
		bool	 debug{ false };			// debug = debug | !debug = release
		bool	 dev{ true };				// development = development | !development = production

		std::string build_type;				// build type (debug, release)
		std::string mode;				// build mode (development, production)
		std::string current_build_cmake_target;		// current build target game-client | game-server | engine-client | engine-server

		// system params

		std::string output_path;
		std::string project_dir;
	};

	uint32_t parse_version( const std::string_view& version_str )
	{
		llvm::Regex				version_pattern( R"((\d+)\.(\d+)\.(\d+))" );
		llvm::SmallVector< llvm::StringRef, 4 > matches;

		if ( version_pattern.match( version_str, &matches ) ) {
			int major = std::stoi( matches[ 1 ].str( ) );
			int minor = std::stoi( matches[ 2 ].str( ) );
			int patch = std::stoi( matches[ 3 ].str( ) );

			return VERSION_PACK( major, minor, patch );
		}

		return VERSION_PACK( 0, 0, 0 );
	}

	Project parse( const json& j )
	{
		Project project;

		const auto& j_project = j[ "project" ];

		project.name	    = j_project[ "name" ].as< std::string >( );
		project.desc	    = j_project[ "desc" ].as< std::string >( );
		project.output_path = j_project[ "output-path" ].as< std::string >( );
		project.project_dir = j_project[ "project-dir" ].as< std::string >( );
		project.version	    = parse_version( j_project[ "version" ].as< std::string >( ) );
		project.debug	    = j_project[ "debug" ].as< bool >( );
		project.dev	    = j_project[ "dev" ].as< bool >( );

		return project;
	}

	void appendProjectNamespace( ASTContext& ctx, const Project& p, NamespaceDecl* ns )
	{
		auto str_qt = TypeBuilder( ctx ).GetType( "string" );
		createVar( ctx, ns, "name", str_qt, TypeBuilder( ctx ).BuildInitStatement( TypeBuilder::Types::string, p.name ) );
		createVar( ctx, ns, "description", str_qt, TypeBuilder( ctx ).BuildInitStatement( TypeBuilder::Types::string, p.desc ) );
		if ( !opt::NoGit )
			createVar( ctx, ns, "git_hash", str_qt, TypeBuilder( ctx ).BuildInitStatement( TypeBuilder::Types::string, p.git_hash ) );
		createVar( ctx,
			   ns,
			   "version",
			   TypeBuilder( ctx ).GetType( "u32" ),
			   TypeBuilder( ctx ).BuildInitStatement( TypeBuilder::Types::i32, std::to_string( p.version ) ) );
		createVar( ctx,
			   ns,
			   "debug",
			   TypeBuilder( ctx ).GetType( "boolean" ),
			   TypeBuilder( ctx ).BuildInitStatement( TypeBuilder::Types::boolean, p.debug ? "true" : "false" ) );
		createVar( ctx,
			   ns,
			   "release",
			   TypeBuilder( ctx ).GetType( "boolean" ),
			   TypeBuilder( ctx ).BuildInitStatement( TypeBuilder::Types::boolean, p.debug ? "false" : "true" ) );
		createVar( ctx,
			   ns,
			   "development",
			   TypeBuilder( ctx ).GetType( "boolean" ),
			   TypeBuilder( ctx ).BuildInitStatement( TypeBuilder::Types::boolean, p.dev ? "true" : "false" ) );
		createVar( ctx,
			   ns,
			   "production",
			   TypeBuilder( ctx ).GetType( "boolean" ),
			   TypeBuilder( ctx ).BuildInitStatement( TypeBuilder::Types::boolean, p.dev ? "false" : "true" ) );
		createVar( ctx,
			   ns,
			   "target",
			   str_qt,
			   TypeBuilder( ctx ).BuildInitStatement( TypeBuilder::Types::string, p.current_build_cmake_target ) );
		createVar( ctx, ns, "system", str_qt, TypeBuilder( ctx ).BuildInitStatement( TypeBuilder::Types::string, opt::TargetSystem ) );
		createVar( ctx, ns, "arch", str_qt, TypeBuilder( ctx ).BuildInitStatement( TypeBuilder::Types::string, opt::TargetArch ) );
		createVar( ctx, ns, "mode", str_qt, TypeBuilder( ctx ).BuildInitStatement( TypeBuilder::Types::string, p.mode ) );
		createVar( ctx, ns, "type", str_qt, TypeBuilder( ctx ).BuildInitStatement( TypeBuilder::Types::string, p.build_type ) );
	}

	void parseJsonObject( const json& root, ASTContext& ctx, NamespaceDecl* ns )
	{

		if ( root.is_object( ) ) {
			for ( const auto& item : root.object_range( ) ) {
				auto	    key = std::string( item.key( ) );
				const auto& val = item.value( );
				if ( val.is_object( ) || val.is_array( ) ) {
					NamespaceDecl* child_ns = CreateNamespace( key, ctx, ns );
					parseJsonObject( val, ctx, child_ns );
					ns->addDecl( child_ns );    // Добавляем декларант в родительский namespace
				} else {
					TypeBuilder::Types tp = TypeBuilder::Types::none;

					switch ( val.type( ) ) {
						case jsoncons::json_type::bool_value	   : tp = TypeBuilder::Types::boolean; break;
						case jsoncons::json_type::string_value	   : tp = TypeBuilder::Types::string; break;
						case jsoncons::json_type::byte_string_value: tp = TypeBuilder::Types::string; break;
						case jsoncons::json_type::int64_value	   : tp = TypeBuilder::Types::i64; break;
						case jsoncons::json_type::uint64_value	   : tp = TypeBuilder::Types::u64; break;
						case jsoncons::json_type::double_value	   : tp = TypeBuilder::Types::f64; break;
						default					   : break;
					}

					for ( auto pos = key.find( '-' ); pos != std::string::npos; pos = key.find( '-' ) ) key[ pos ] = '_';

					createVar( ctx,
						   ns,
						   key,
						   TypeBuilder( ctx ).GetType( tp ),
						   TypeBuilder( ctx ).BuildInitStatement( tp,
											  val.as_string( ) ) );	   // Вызов пользовательской функции
				}
			}
		}
	}

}    // namespace ConfParser

void copytight_show( llvm::raw_ostream& os )
{
	os << R"(// This file was automatically generated by cth++ from a JSON configuration.
// DO NOT MODIFY THIS FILE MANUALLY.
//
// Copyright 2024 olokreZ ( Долгополов Василий Васильевич | Dolgopolov Vasily Vasilievich )
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
)";
	os << "\n\n";
}

#include <stacktrace>

#ifdef WIN32
#	include <windows.h>

// Переводчик SEH в C++ исключение

void __se_translator( uint32_t code, EXCEPTION_POINTERS* pExp )

{
	std::string exceptionMessage;

	switch ( code ) {
		case EXCEPTION_ACCESS_VIOLATION:
			exceptionMessage = "Access violation exception occurred!";
			exceptionMessage += "\nAccess Type: " + std::to_string( pExp->ExceptionRecord->ExceptionInformation[ 0 ] );
			exceptionMessage += "\nAccess Address: " + std::to_string( ( uintptr_t )pExp->ExceptionRecord->ExceptionInformation[ 1 ] );
			break;

		case EXCEPTION_ARRAY_BOUNDS_EXCEEDED: exceptionMessage = "Array bounds exceeded exception occurred!"; break;

		case EXCEPTION_DATATYPE_MISALIGNMENT: exceptionMessage = "Datatype misalignment exception occurred!"; break;

		case EXCEPTION_FLT_DENORMAL_OPERAND : exceptionMessage = "Floating-point denormal operand exception occurred!"; break;

		case EXCEPTION_FLT_DIVIDE_BY_ZERO   : exceptionMessage = "Floating-point divide by zero exception occurred!"; break;

		case EXCEPTION_FLT_INEXACT_RESULT   : exceptionMessage = "Floating-point inexact result exception occurred!"; break;

		case EXCEPTION_FLT_INVALID_OPERATION: exceptionMessage = "Floating-point invalid operation exception occurred!"; break;

		case EXCEPTION_FLT_OVERFLOW	    : exceptionMessage = "Floating-point overflow exception occurred!"; break;

		case EXCEPTION_FLT_STACK_CHECK	    : exceptionMessage = "Floating-point stack check exception occurred!"; break;

		case EXCEPTION_FLT_UNDERFLOW	    : exceptionMessage = "Floating-point underflow exception occurred!"; break;

		case EXCEPTION_ILLEGAL_INSTRUCTION  : exceptionMessage = "Illegal instruction exception occurred!"; break;

		case EXCEPTION_IN_PAGE_ERROR:
			exceptionMessage = "In-page error exception occurred!";
			exceptionMessage += "\nAccess Type: " + std::to_string( pExp->ExceptionRecord->ExceptionInformation[ 0 ] );
			exceptionMessage += "\nFaulting Address: " + std::to_string( ( uintptr_t )pExp->ExceptionRecord->ExceptionInformation[ 1 ] );
			exceptionMessage += "\nNTSTATUS Code: " + std::to_string( pExp->ExceptionRecord->ExceptionInformation[ 2 ] );
			break;

		case EXCEPTION_INT_DIVIDE_BY_ZERO      : exceptionMessage = "Integer divide by zero exception occurred!"; break;

		case EXCEPTION_INT_OVERFLOW	       : exceptionMessage = "Integer overflow exception occurred!"; break;

		case EXCEPTION_INVALID_DISPOSITION     : exceptionMessage = "Invalid disposition exception occurred!"; break;

		case EXCEPTION_NONCONTINUABLE_EXCEPTION: exceptionMessage = "Noncontinuable exception occurred!"; break;

		case EXCEPTION_PRIV_INSTRUCTION	       : exceptionMessage = "Privileged instruction exception occurred!"; break;

		case EXCEPTION_SINGLE_STEP	       : exceptionMessage = "Single step exception occurred!"; break;

		case EXCEPTION_STACK_OVERFLOW	       : exceptionMessage = "Stack overflow exception occurred!"; break;
	}

	exceptionMessage += "\n" + std::to_string( std::stacktrace::current( ) );

	throw std::runtime_error( exceptionMessage );
}

#endif

int main( int argc, char** argv )
{

	_set_se_translator( &__se_translator );

	const auto cbVersion = []( auto& os ) { os << "cth++ version 1.0.0\n"; };
	cl::SetVersionPrinter( cbVersion );
	cl::AddExtraVersionPrinter( cbVersion );

	cl::HideUnrelatedOptions( opt::CthOption );

	cl::ParseCommandLineOptions( argc, argv );

	switch ( common::const_hash( opt::TargetArch ) ) {
		case common::const_hash( "x86" ): opt::TargetArch = "x86"; break;
		case common::const_hash( "x64" ):
		default				: opt::TargetArch = "x64"; break;
	}

	CompilerInstance*    ci		  = createCompilerInstance( );
	ASTContext&	     context	  = ci->getASTContext( );
	TranslationUnitDecl* global_scope = context.getTranslationUnitDecl( );

	// Создание пространства имен
	NamespaceDecl* ns_global_config = CreateNamespace( opt::GlobalNamespace, context, global_scope );

	NamespaceDecl* namespaceProject = CreateNamespace( "project", context, ns_global_config );

	ConfParser::Project proj;

	try {

		std::ifstream file( opt::ConfigFile );
		if ( !file ) {
			llvm::errs( ) << "Error: file not found: " << opt::ConfigFile;
			return -1;
		}
		auto json = json::parse( file );
		proj	  = ConfParser::parse( json );

		if ( !opt::WorkingDir.empty( ) ) proj.project_dir = opt::WorkingDir;
		if ( proj.project_dir.empty( ) ) {
			llvm::SmallVector< char, 256 > path_data_raw;
			const auto		       errc = llvm::sys::fs::current_path( path_data_raw );
			proj.project_dir		    = { path_data_raw.data( ), path_data_raw.size( ) };
		}

		if ( opt::Debug || opt::Release ) proj.debug = opt::Debug && !opt::Release;
		if ( opt::Development || opt::Production ) proj.dev = opt::Development && !opt::Production;
		if ( !opt::OutputPath.empty( ) ) proj.output_path = opt::OutputPath;

		proj.current_build_cmake_target = opt::CmakeTarget;

		proj.build_type = proj.debug ? "debug" : "release";
		proj.mode	= proj.dev ? "development" : "production";
		proj.git_hash	= getHashGitCommit( proj.project_dir );

		if ( !proj.git_hash.empty( ) ) proj.git_hash.pop_back( );

		ConfParser::appendProjectNamespace( context, proj, namespaceProject );

		ns_global_config->addDecl( namespaceProject );

		ConfParser::parseJsonObject( json[ "config" ], context, ns_global_config );

		global_scope->addDecl( ns_global_config );

		// Вывод сгенерированного кода
		LangOptions langOpts;

		using e_lang_t = FIX8::conjure_enum< LangStandard::Kind >;

		langOpts.LangStd = LangStandard::lang_cxx23;

		if ( auto lang = e_lang_t::unscoped_string_to_enum( "lang_" + opt::Std ); lang ) langOpts.LangStd = *lang;

		PrintingPolicy policy( langOpts );
		policy.Bool    = 1;
		policy.MSWChar = 1;

		using namespace srilakshmikanthanp;

		const auto execdir = std::filesystem::path( llvm::sys::fs::getMainExecutable( *argv, &main ) ).parent_path( );

		libfiglet::figlet figlet_copyright( libfiglet::flf_font::make_shared( ( execdir / "Ivrit.flf" ).string( ) ),
						    libfiglet::full_width::make_shared( ) );

		libfiglet::figlet figlet_ProjectLogo( libfiglet::flf_font::make_shared( ( execdir / "Standard.flf" ).string( ) ),
						      libfiglet::full_width::make_shared( ) );

		// std::string		 config_impl;
		// llvm::raw_string_ostream os( config_impl );

		llvm::sys::fs::create_directories( std::filesystem::path( proj.output_path ).parent_path( ).string( ) );

		std::error_code	    ec;
		llvm::raw_fd_stream os( proj.output_path, ec );

		if ( !opt::NoLogo ) llvm::outs( ) << "\n" << figlet_ProjectLogo( "cth++" ) << "\n";

		copytight_show( os );

		os << "/*\n";
		os << figlet_ProjectLogo( proj.name ) << "\n";
		os << "*/\n\n";

		os << "#pragma once\n\n";

		os << "#define VERSION_PACK(MAJOR, MINOR, PATCH) ( ( ( MAJOR ) << 16 ) | ( ( MINOR ) << 8 ) | ( PATCH ) )"
		   << "\n\n\n";

		global_scope->print( os, policy );

		if ( opt::RewriteConfig ) {
			auto jp		    = json[ "project" ];
			jp[ "working-dir" ] = proj.project_dir;
			jp[ "output-path" ] = proj.output_path;
			jp[ "debug" ]	    = proj.debug;
			jp[ "dev" ]	    = proj.dev;

			std::ofstream of( opt::ConfigFile, std::ios::out | std::ios::binary | std::ios::trunc );
			// of << json;
			json.dump( of, true );
			of.flush( );
			of.close( );
		}

		delete ci;

	} catch ( const std::exception& e ) {
		llvm::errs( ) << "Error: " << e.what( ) << "\n";
		llvm::errs( ) << "stack trace:\n";
		std::cout << std::stacktrace::current( ) << "\n";
		return -1;
	}

	return 0;
}
