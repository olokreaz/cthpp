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

//
// Created by @olokreaz on 18.08.2024.
//

#ifndef PROGRAM_OPTIONS_HPP
#define PROGRAM_OPTIONS_HPP

#include <llvm/Support/CommandLine.h>
namespace cl = llvm::cl;
static cl::OptionCategory CthOption( "cth++ options" );
static cl::opt< std::string >
		optOutputPath( "output", cl::desc( "Output path" ), cl::value_desc( "path" ), cl::init( "./conf.hpp" ), cl::cat( CthOption ) );
static cl::opt< std::string > optConfigFile( "config",
					     cl::desc( "Path to the JSON configuration file" ),
					     cl::value_desc( "path" ),
					     cl::init( "./conf.json" ),
					     cl::cat( CthOption ) );

static cl::opt< std::string > optWorkingDir( "working", cl::desc( "Set the project directory" ), cl::value_desc( "path" ), cl::cat( CthOption ) );

static cl::opt< bool > optDebug( "dbg", cl::desc( "Set build mode to debug" ), cl::cat( CthOption ) );
static cl::opt< bool > optRelease( "rel", cl::desc( "Set build mode to release" ), cl::cat( CthOption ) );

static cl::opt< bool > optDevelopment( "development", cl::desc( "Set build mode to development" ), cl::init( false ), cl::cat( CthOption ) );
static cl::opt< bool > optProduction( "prodaction", cl::desc( "Set build mode to production" ), cl::init( false ), cl::cat( CthOption ) );

static cl::opt< std::string > optGlobalNamespace( "namespace",
						  cl::desc( "Set the global namespace name" ),
						  cl::value_desc( "name" ),
						  cl::init( "config" ),
						  cl::cat( CthOption ) );

static cl::opt< std::string > optCmakeTarget( "cmake-target-current-build",
					      cl::desc( "Specify the current build target" ),
					      cl::value_desc( "target" ),
					      cl::init( "none" ),
					      cl::cat( CthOption ) );

static cl::opt< std::string >
		optStd( "std", cl::desc( "Specify the C++ standard" ), cl::value_desc( "cxx standard" ), cl::init( "std23" ), cl::cat( CthOption ) );

static cl::opt< bool > optRewriteConfig( "rewrite", cl::desc( "Rewrite the configuration file" ), cl::init( false ), cl::cat( CthOption ) );
static cl::opt< bool > optNoLogo( "no-logo", cl::desc( "Disable logo" ), cl::init( false ), cl::cat( CthOption ) );

static cl::opt< bool > optNoGit( "no-git", cl::desc( "Disable git hash" ), cl::init( false ), cl::cat( CthOption ) );
#endif	  //PROGRAM_OPTIONS_HPP
