cmake_minimum_required( VERSION 3.22 )
project( GenerationCompileTimeConfig )

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
		/arch:AVX
		/openmp
		/FS
		
		${RELEASE_COMPILE_OPTIONS}
		)
elseif ( ${LINUX} )
	add_compile_options( -finput-charset=UTF-8
	                     -fexceptions
	                     -fstack-protector
	                     -frtti
	                     -fexceptions
	                     -Wno-deprecated-declarations
	                     -mavx
	                     -fopenmp )
endif ()

add_compile_definitions(
	UNICODE
	_UNICODE
	_CONSOLE
	NOMINMAX
	)

add_executable( configure
                src/main.cpp )


find_package( ctre CONFIG REQUIRED )
find_package( fmt CONFIG REQUIRED )
find_package( argh CONFIG REQUIRED )
find_package( nlohmann_json CONFIG REQUIRED )
find_package( hjson CONFIG REQUIRED )
find_package( mstch CONFIG REQUIRED )

find_package( LLVM CONFIG REQUIRED )
list( APPEND CMAKE_MODULE_PATH "${LLVM_CMAKE_DIR}" )
include( HandleLLVMOptions )
add_definitions( ${LLVM_DEFINITIONS} )
target_include_directories( configure PRIVATE ${LLVM_INCLUDE_DIRS} )

llvm_map_components_to_libnames( llvm_libs support core tablegen )

message( STATUS "LLVM_LIBS: ${llvm_libs}" )

target_link_libraries( configure PRIVATE ${llvm_libs} )


target_link_libraries( configure PRIVATE
                       argh
                       ctre::ctre
                       mstch::mstch
                       fmt::fmt-header-only
                       hjson
                       )

# this is heuristically generated, and may not be correct


