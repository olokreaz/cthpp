#pragma once
#include "config_type.hpp"

class Config
{
	[[nodiscard]] Logic::ConfigValue::ConfigType	    _parseString( const std::string& str ) const;
	std::unordered_map< std::string, Logic::Namespace > m_namespaces;
	Logic::Namespace				    m_global_namespace;

public:
	explicit Config( const std::string& global_namespace_name ) : m_global_namespace( global_namespace_name )
	{
	}

	void addNamespace( const std::string& name )
	{
		m_namespaces.emplace( name, Logic::Namespace( name ) );
	}

	void addNamespace( Logic::Namespace&& namespace_ )
	{
		m_namespaces.emplace( namespace_.name( ), std::move( namespace_ ) );
	}

	void addConfigValue( Logic::ConfigValue )
	{
	}
};
struct GLobalState {
	std::string ProjectName;
	std::string ProjectDir;
	std::string ProjectFile;
	std::string WorkingDir;
	std::string ConfigFile;
	Config	    config;
};

extern GLobalState g_state;
