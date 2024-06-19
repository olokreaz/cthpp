//
// Created by @olokreaz on 19.06.2024.
//

#pragma once
#ifndef HEADERGENERATOR_HPP
#	define HEADERGENERATOR_HPP

#	include <string>
#	include <unordered_map>
#	include <vector>

struct Variable {
	std::string type;
	std::string name;
	std::string value;
};

struct Namespace {
	std::string				     name;
	std::vector< Variable >			     variables;
	std::unordered_map< std::string, Namespace > namespaces;
};

class HeaderGenerator
{
public:
	HeaderGenerator( const std::string& copyright, const std::string& filename );

	void addNamespace( const std::string& name );
	void addVariable( const std::string& namespaceName, const std::string& type, const std::string& name, const std::string& value );
	void generate( ) const;

private:
	std::string filename;
	std::string copyright;
	Namespace   rootNamespace;

	void generateNamespace( std::ofstream& file, const Namespace& ns, const std::string& indent ) const;
};
#endif //HEADERGENERATOR_HPP
