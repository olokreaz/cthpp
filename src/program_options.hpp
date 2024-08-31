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

namespace opt {

	static cl::OptionCategory     CthOption( "cth++ options" );
	static cl::opt< std::string > ConfigFile( "config",
						  cl::desc( "Path to the JSON configuration file" ),
						  cl::value_desc( "path" ),
						  cl::Required,
						  cl::cat( CthOption ) );

	static cl::opt< std::string >
			OutputPath( "output", cl::desc( "Output path" ), cl::value_desc( "path" ), cl::init( "./conf.hpp" ), cl::cat( CthOption ) );

	static cl::opt< std::string >
			WorkingDir( "working-dir", cl::desc( "Set the project directory" ), cl::value_desc( "path" ), cl::cat( CthOption ) );

	static cl::opt< bool > Debug( "dbg", cl::desc( "Set build mode to debug" ), cl::cat( CthOption ) );

	static cl::opt< bool > Release( "rel", cl::desc( "Set build mode to release" ), cl::cat( CthOption ) );

	static cl::opt< bool > Development( "dev", cl::desc( "Set build mode to development" ), cl::cat( CthOption ) );
	static cl::opt< bool > Production( "prod", cl::desc( "Set build mode to production" ), cl::cat( CthOption ) );

	static cl::opt< std::string > GlobalNamespace( "namespace",
						       cl::desc( "Set the global namespace name" ),
						       cl::value_desc( "name" ),
						       cl::init( "config" ),
						       cl::cat( CthOption ) );

	static cl::opt< std::string > CmakeTarget( "cmake-target-current-build",
						   cl::desc( "Specify the current build target" ),
						   cl::value_desc( "target" ),
						   cl::init( "none" ),
						   cl::cat( CthOption ) );

	static cl::opt< std::string > TargetSystem( "target-system",
						    cl::desc( "Specify the target system" ),
						    cl::value_desc( "system" ),
						    cl::init( "none" ),
						    cl::cat( CthOption ) );

	static cl::opt< std::string > TargetArch( "target-arch",
						  cl::desc( "Specify the target architecture" ),
						  cl::value_desc( "arch" ),
						  cl::init( "x64" ),
						  cl::cat( CthOption ) );

	static cl::opt< std::string > Std( "std",
					   cl::desc( "Specify the C++ standard" ),
					   cl::value_desc( "cxx standard" ),
					   cl::init( "std23" ),
					   cl::cat( CthOption ) );

	static cl::opt< bool >
			RewriteConfig( "rewrite-config", cl::desc( "Rewrite the configuration file" ), cl::init( false ), cl::cat( CthOption ) );

	static cl::opt< bool > NoLogo( "no-logo", cl::desc( "Disable logo" ), cl::init( false ), cl::cat( CthOption ) );

	static cl::opt< bool > NoGit( "no-git", cl::desc( "Disable git hash" ), cl::init( false ), cl::cat( CthOption ) );

	static cl::opt< bool > CreateConfig( "create", cl::desc( "Create a new configuration file" ), cl::init( false ) );
	static cl::opt< bool > WithCmake( "cmake", cl::desc( "Include CMake target in the configuration file" ), cl::init( false ) );
}    // namespace opt

#endif	  //PROGRAM_OPTIONS_HPP
