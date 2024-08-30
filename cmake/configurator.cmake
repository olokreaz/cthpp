	find_programm( CTHPP "cth++" )
	
	if ( NOT CTHPP )
		message( FATAL_ERROR " cthpp not found" )
	endif ()
	
	message( STATUS "cthpp found at ${CTHPP}" )
	
	
	function ( add_target_config )
		set( options CONFIG NAMESPACE WORKING_DIR TYPE MODE TARGET OUTPUT )
		cmake_parse_arguments( CONFIG "" "${options}" "" ${ARGN} )
		
		if ( CONFIG_OUTPUT )
			message( STATUS "OUTPUT argument is ${CONFIG_OUTPUT}" )
		else ()
			message( FATAL_ERROR "OUTPUT argument is required" )
		endif ()
		
		if ( NOT CONFIG_CONFIG )
			message( FATAL_ERROR "CONFIG argument is required" )
			if ( NOT EXISTS ${CONFIG_CONFIG} )
				message( FATAL_ERROR "Config file ${CONFIG_CONFIG} does not exist" )
			endif ()
		endif ()
		
		if ( CONFIG_NAMESPACE )
			message( STATUS "NAMESPACE argument is ${CONFIG_NAMESPACE}" )
		else ()
			set( CONFIG_NAMESPACE "config" )
			message( STATUS "NAMESPACE default config" )
		endif ()
		
		if ( CONFIG_WORKING_DIR )
			message( STATUS "WORKING_DIR argument is ${CONFIG_WORKING_DIR}" )
		else ()
			set( CONFIG_WORKING_DIR ${CMAKE_CURRENT_SOURCE_DIR} )
			message( STATUS "WORKING_DIR default ${CMAKE_CURRENT_SOURCE_DIR}" )
		endif ()
		
		# Обработка TYPE
		if ( CONFIG_TYPE )
			if ( CONFIG_TYPE STREQUAL "DEBUG" )
				set( TYPE_FLAG "--dev" )
			elseif ( CONFIG_TYPE STREQUAL "RELEASE" )
				set( TYPE_FLAG "--rel" )
			else ()
				message( FATAL_ERROR "Invalid TYPE argument: ${CONFIG_TYPE}. Expected DEBUG or RELEASE." )
			endif ()
			message( STATUS "TYPE argument is ${CONFIG_TYPE} ( ${TYPE_FLAG} ) " )
		else ()
			set( CONFIG_TYPE ${CMAKE_BUILD_TYPE} )
			if ( CONFIG_TYPE STREQUAL "Debug" )
				set( TYPE_FLAG "--dbg" )
			elseif ( CONFIG_TYPE STREQUAL "Release" )
				set( TYPE_FLAG "--rel" )
			endif ()
			message( STATUS "TYPE default DEBUG ( ${TYPE_FLAG} ) " )
		endif ()
		
		# Обработка MODE
		if ( CONFIG_MODE )
			if ( CONFIG_MODE STREQUAL "DEVELOPMENT" )
				set( MODE_FLAG "--development" )
			elseif ( CONFIG_MODE STREQUAL "PRODUCTION" )
				set( MODE_FLAG "--production" )
			else ()
				message( FATAL_ERROR "Invalid MODE argument: ${CONFIG_MODE}. Expected DEVELOPMENT or PRODUCTION." )
			endif ()
			message( STATUS "MODE argument is ${CONFIG_MODE} ( ${MODE_FLAG} ) " )
		else ()
			set( CONFIG_MODE " DEVELOPMENT" )
			set( MODE_FLAG "--development" )
			message( STATUS "MODE default DEVELOPMENT ( ${MODE_FLAG} ) " )
		endif ()
		
		if ( CONFIG_TARGET )
			message( STATUS " TARGET argument is ${CONFIG_TARGET}" )
		else ()
			message( FATAL_ERROR "TARGET argument is required" )
		endif ()
		
		set( OUT "${CMAKE_CURRENT_BINARY_DIR}/cthpp/${CONFIG_TARGET}" )
		
		execute_process( COMMAND ${CTHPP} --config=${CONFIG_CONFIG} --namespace=${CONFIG_NAMESPACE} --cmake-target-current-build=${CONFIG_TARGET} --working=${CONFIG_WORKING_DIR} ${TYPE_FLAG} ${MODE_FLAG} --output=${OUT}/${CONFIG_OUTPUT} --rewrite --no-logo RESULT_VARIABLE result OUTPUT_VARIABLE output )
		
		target_include_directories( ${CONFIG_TARGET} PRIVATE ${OUT} )
		
		if ( output )
			message( STATUS "${output}" )
		endif ()
		
		if ( NOT result EQUAL "0" )
			message( FATAL_ERROR "Build failed with error code: ${result}" )
		endif ()
	
	
	endfunction ()