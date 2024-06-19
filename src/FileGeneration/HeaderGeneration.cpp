//
// Created by @olokreaz on 19.06.2024.
//
#include <olokreaz/FileGeneration/HeaderGenerator.hpp>
#include <algorithm>
#include <fstream>
#include <ranges>
#include <sstream>
#include <string>

HeaderGenerator::HeaderGenerator( const std::string& copyright, const std::string& filename ) : copyright( copyright ), filename( filename )
{
	rootNamespace.name = "global";
}

void HeaderGenerator::addNamespace( const std::string& name )
{
	rootNamespace.namespaces[ name ] = Namespace{ name, { }, {} };
}

void HeaderGenerator::addVariable( const std::string& namespaceName, const std::string& type, const std::string& name, const std::string& value )
{
	rootNamespace.namespaces[ namespaceName ].variables.push_back( Variable{ type, name, value } );
}

void HeaderGenerator::generate( ) const
{
	std::ofstream file( filename );
	std::string   guardName = filename;
	std::ranges::replace( guardName, '.', '_' );
	std::ranges::transform( guardName, guardName.begin( ), ::toupper );

	// Write copyright
	file << "//\n// " << copyright << "\n//\n\n";

	// Write include guards
	file << "#pragma once\n\n";
	file << "#ifndef " << guardName << "\n";
	file << "#define " << guardName << "\n\n";

	// Generate namespaces and variables
	for ( const auto& [ nsName, ns ] : rootNamespace.namespaces ) generateNamespace( file, ns, "" );

	// Close include guards
	file << "#endif // " << guardName << "\n";
}

void HeaderGenerator::generateNamespace( std::ofstream& file, const Namespace& ns, const std::string& indent ) const
{
	file << indent << "namespace " << ns.name << " {\n";
	std::string newIndent = indent + "    ";

	for ( const auto& var : ns.variables ) file << newIndent << "constexpr " << var.type << " " << var.name << " = " << var.value << ";\n";

	for ( const auto& [ subNsName, subNs ] : ns.namespaces ) generateNamespace( file, subNs, newIndent );

	file << indent << "}\n";
}
