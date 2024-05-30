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
                src/main.cpp
                )

# this is heuristically generated, and may not be correct
find_package( ctre CONFIG REQUIRED )
find_package( alpaca CONFIG REQUIRED )
find_package( fmt CONFIG REQUIRED )
find_package( ftxui CONFIG REQUIRED )
find_package( argh CONFIG REQUIRED )
target_link_libraries( configure PRIVATE
                       argh
                       ctre::ctre
                       alpaca::alpaca
                       fmt::fmt
                       ftxui::dom ftxui::screen ftxui::component
                       )

# this is heuristically generated, and may not be correct


