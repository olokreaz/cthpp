#include "olokreaz/FileGeneration/HeaderGenerator.hpp"
#include <Hjson/hjson.h>
#include <fmt/core.h>
#include <fmt/format.h>
#include <fmt/os.h>
#include <algorithm>
#include <fstream>
#include <ranges>
#include <sstream>

/**
 * @brief Constructs a HeaderGenerator.
 * @param copyright Copyright notice for the generated header file.
 * @param filename Name of the output header file.
 */
HeaderGenerator::HeaderGenerator( const std::string& copyright, const std::string& filename ) : copyright( copyright ), filename( filename )
{
	rootNamespace.name = "global";
}

/**
 * @brief Adds a namespace to the generator.
 * @param name Name of the namespace to add.
 */
void HeaderGenerator::addNamespace( const std::string& name )
{
	rootNamespace.namespaces[ name ] = Namespace{ name, { }, {} };
}

/**
 * @brief Adds a variable to a namespace.
 * @param namespaceName Name of the namespace.
 * @param type Type of the variable.
 * @param name Name of the variable.
 * @param value Value of the variable.
 * @param dep Optional dependency (mode or type).
 * @param depValues Map of values based on the dependency.
 */
void HeaderGenerator::addVariable( const std::string&					 namespaceName,
				   const std::string&					 type,
				   const std::string&					 name,
				   const std::string&					 value,
				   const std::optional< std::string >&			 dep,
				   const std::unordered_map< std::string, std::string >& depValues )
{
	rootNamespace.namespaces[ namespaceName ].variables.push_back( Variable{ type, name, value, dep, depValues } );
}

/**
 * @brief Generates the header file.
 */
void HeaderGenerator::generate( ) const
{
	auto	    out	      = fmt::output_file( filename );
	std::string guardName = filename;
	std::replace( guardName.begin( ), guardName.end( ), '.', '_' );
	std::transform( guardName.begin( ), guardName.end( ), guardName.begin( ), ::toupper );

	// Write copyright
	out.print( "//\n// {}\n//\n\n", copyright );

	// Write include guards
	out.print( "#pragma once\n\n" );
	out.print( "#ifndef {}\n", guardName );
	out.print( "#define {}\n\n", guardName );

	// Generate namespaces and variables
	for ( const auto& [ nsName, ns ] : rootNamespace.namespaces ) generateNamespace( out, ns, "" );

	// Close include guards
	out.print( "#endif // {}\n", guardName );
}

/**
 * @brief Generates code for a namespace and its contents.
 * @param out Output stream.
 * @param ns Namespace to generate.
 * @param indent Indentation string.
 */
void HeaderGenerator::generateNamespace( fmt::ostream& out, const Namespace& ns, const std::string& indent ) const
{
	out.print( "{}namespace {} {{\n", indent, ns.name );
	std::string newIndent = indent + "    ";

	for ( const auto& var : ns.variables ) {
		std::string resolvedValue = var.dep ? resolveDepValue( var ) : var.value;
		out.print( "{}constexpr {} {} = {};\n", newIndent, var.type, var.name, resolvedValue );
	}

	for ( const auto& [ subNsName, subNs ] : ns.namespaces ) generateNamespace( out, subNs, newIndent );

	out.print( "{}}}\n", indent );
}

/**
 * @brief Parses an HJSON file to configure namespaces and variables.
 * @param hjsonFilename Path to the HJSON file.
 */
void HeaderGenerator::parseHjson( const std::string& hjsonFilename )
{
	Hjson::Value hjsonRoot = Hjson::UnmarshalFromFile( hjsonFilename.c_str( ) );

	if ( hjsonRoot.type( ) == Hjson::Type::Map ) {
		// Process the "project" section
		if ( hjsonRoot[ "project" ].type( ) == Hjson::Type::Map ) {
			Namespace projectNamespace{ "project", { }, {} };
			processHjsonObject( projectNamespace, hjsonRoot[ "project" ] );
			rootNamespace.namespaces[ "config::project" ] = projectNamespace;

			// Check for mode or type
			if ( hjsonRoot[ "project" ][ "mode" ].defined( ) ) projectMode = hjsonRoot[ "project" ][ "mode" ].to_string( );
			if ( hjsonRoot[ "project" ][ "type" ].defined( ) ) projectType = hjsonRoot[ "project" ][ "type" ].to_string( );
		}

		// Process the "config" section
		if ( hjsonRoot[ "config" ].type( ) == Hjson::Type::Map ) {
			for ( const auto& kv : hjsonRoot[ "config" ] ) {
				const std::string&  key	  = kv.first;
				const Hjson::Value& value = kv.second;
				Namespace	    configNamespace{ key, { }, {} };
				processHjsonObject( configNamespace, value );
				rootNamespace.namespaces[ "config::" + key ] = configNamespace;
			}
		}
	}

	// Apply mode or type specific configuration adjustments
	applyDepConfig( );
}

/**
 * @brief Processes an HJSON object into a namespace.
 * @param currentNamespace Current namespace being processed.
 * @param obj HJSON object.
 */
void HeaderGenerator::processHjsonObject( Namespace& currentNamespace, const Hjson::Value& obj )
{
	if ( obj.type( ) == Hjson::Type::Map ) {
		for ( const auto& kv : obj ) {
			const std::string&  key	  = kv.first;
			const Hjson::Value& value = kv.second;

			if ( value.type( ) == Hjson::Type::Map ) {
				if ( value[ "dep" ].defined( ) ) {
					std::unordered_map< std::string, std::string > depValues;
					for ( const auto& depKv : value )
						if ( depKv.first != "dep" ) depValues[ depKv.first ] = depKv.second.to_string( );
					currentNamespace.variables.push_back( Variable{ "char*", key, "", value[ "dep" ].to_string( ), depValues } );
				} else {
					Namespace newNamespace{ key, { }, {} };
					processHjsonObject( newNamespace, value );
					currentNamespace.namespaces[ key ] = newNamespace;
				}
			} else {
				currentNamespace.variables.push_back( Variable{ "char*", key, value.to_string( ), std::nullopt, {} } );
			}
		}
	}
}

/**
 * @brief Applies dependency-based configurations to variables.
 */
void HeaderGenerator::applyDepConfig( )
{
	for ( auto& [ name, variables, namespaces ] : rootNamespace.namespaces | std::views::values ) {
		for ( auto& var : variables )
			if ( var.dep ) var.value = resolveDepValue( var );
		for ( auto& subNs : namespaces | std::views::values ) applyDepConfigToNamespace( subNs );
	}
}

/**
 * @brief Applies dependency-based configurations to variables within a namespace.
 * @param ns Namespace to process.
 */
void HeaderGenerator::applyDepConfigToNamespace( Namespace& ns )
{
	for ( auto& var : ns.variables )
		if ( var.dep ) var.value = resolveDepValue( var );
	for ( auto& subNs : ns.namespaces | std::views::values ) applyDepConfigToNamespace( subNs );
}

/**
 * @brief Resolves the value of a variable based on its dependency.
 * @param var Variable to resolve.
 * @return Resolved value.
 */
std::string HeaderGenerator::resolveDepValue( const Variable& var ) const
{
	if ( var.dep == "mode" && !projectMode.empty( ) ) {
		if ( var.depValues.find( projectMode ) != var.depValues.end( ) ) return "\"" + var.depValues.at( projectMode ) + "\"";
	} else if ( var.dep == "type" && !projectType.empty( ) ) {
		if ( var.depValues.find( projectType ) != var.depValues.end( ) ) return "\"" + var.depValues.at( projectType ) + "\"";
	}
	return "\"\"";
}
