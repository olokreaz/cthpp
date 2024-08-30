cmake_minimum_required( VERSION 3.22 )
project( cth++ C CXX )

set( CMAKE_CXX_STANDARD 23 )
set( CMAKE_EXPORT_COMPILE_COMMANDS ON )


message( STATUS "~ ~ ~ ~ ~ ${PROJECT_NAME} ~ ~ ~ ~ ~" )

if ( DEV )
	link_directories( $ENV{VCPKG_ROOT}/installed/x64-${TARGET_OS}/debug/lib )
	link_directories( $ENV{VCPKG_ROOT}/installed/x64-${TARGET_OS}/debug/bin )
else ()
	link_directories( $ENV{VCPKG_ROOT}/installed/x64-${TARGET_OS}/lib )
	link_directories( $ENV{VCPKG_ROOT}/installed/x64-${TARGET_OS}/bin )
endif ()

# linker options
if ( ${WIN32} )
	add_link_options(
		/SUBSYSTEM:CONSOLE
		#/FORCE:MULTIPLE
		)
endif ()

# compiler options
if ( ${WIN32} )
	add_compile_options(
		/utf-8
		/MP
		/EHsc
		/GS
		/GF
		/GR
		/GA
		/wd4996
		/openmp
		/FS
		/VERBOSE
		
		)
	
	add_link_options( /VERBOSE )
elseif ( ${LINUX} )
	add_compile_options( -finput-charset=UTF-8
	                     -fexceptions
	                     -fstack-protector
	                     -frtti
	                     -fexceptions
	                     -Wno-deprecated-declarations
	                     -fopenmp )
endif ()

add_compile_definitions(
	UNICODE
	_UNICODE
	_CONSOLE
	NOMINMAX
	
	)

if ( CMAKE_BUILD_TYPE STREQUAL "Debug" )
	add_compile_definitions( _DEBUG DEBUG )
else ()
	add_compile_definitions( NDEBUG )
endif ()

add_executable( ${PROJECT_NAME}
                src/main.cpp
                )

set_target_properties( ${PROJECT_NAME} PROPERTIES
                       MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL"
                       )

find_package( unofficial-libgit2 CONFIG REQUIRED )

find_package( jsoncons CONFIG REQUIRED )

find_package( LLVM CONFIG REQUIRED )
find_package( Clang CONFIG REQUIRED )

list( APPEND CMAKE_MODULE_PATH "${LLVM_CMAKE_DIR}" )
message( STATUS "LLVM_CMAKE_DIR: ${LLVM_CMAKE_DIR}" )

include( HandleLLVMOptions )
add_definitions( ${LLVM_DEFINITIONS} )
target_include_directories( ${PROJECT_NAME} PRIVATE ${LLVM_INCLUDE_DIRS} )

llvm_map_components_to_libnames( llvm_libs
                                 support core irreader analysis
                                 )


target_link_libraries( ${PROJECT_NAME} PRIVATE ${llvm_libs} )

include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/src )

include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/third-party/libfiglet/src )

set( CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL" )


target_link_libraries( ${PROJECT_NAME} PRIVATE
                       jsoncons
                       
                       unofficial::libgit2::libgit2
                       
                       clangTooling
                       clangBasic
                       clangAST
                       clangASTMatchers
                       clangAnalysis
                       clangCodeGen
                       clangDynamicASTMatchers
                       #                       clangFronted
                       clangFormat
                       #                       clangFrontedTool
                       clangHandleCXX
                       clangHandleLLVM
                       clangLex
                       clangParse
                       clangRewrite
                       clangSupport
                       clangToolingSyntax
                       clangTransformer
                       )
