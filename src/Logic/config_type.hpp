#pragma once
#include <ctre.hpp>
#include <functional>

#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include <fui/fui.h>

namespace Logic
{

	struct Dependence {
		enum class DepeneceType : uint8_t {
			kNot	       = 0x10,	 // NotPrefix
			kDefined       = 1 << 0, // defined
			kPlatform      = 1 << 1, // win/linux/etc..
			kTarget	       = 1 << 2, // client/server/master-server/etc...
			kLocalSettings = 1 << 3, // *Virtual Settins* Project -> Settings -> add/remove settings
			// Notter
			kNotDefined	  = kNot | kDefined,	   // execlude defines
			kNotPlatform	  = kNot | kPlatform,	   // execlude platform
			kNotTarget	  = kNot | kTarget,	   // execlude win / execlude linux / execlude etc...
			kNotLocalSettings = kNot | kLocalSettings, // execlude *Virtual Settins* Project -> Settings -> virtual settings
		};
		std::string name;
		std::string value;
		uint8_t	    type;

		explicit Dependence( uint8_t type, std::string name, std::string value )
		    : name( std::move( name ) ),
		      value( std::move( value ) ),
		      type( type )
		{

		}
	};

	struct ConfigValue {
		enum ConfigType : uint8_t { kI8, kU8, kBool, kI16, kU16, kI32, kU32, kFloat, kI64, kU64, kDouble, kString };
		std::string		  name;
		std::string		  value;
		ConfigType		  type;
		std::vector< Dependence > dependences;
		explicit ConfigValue( std::string name, std::string value, ConfigType type, std::initializer_list< Dependence > dependences )
		    : name( std::move( name ) ),
		      value( std::move( value ) ),
		      type( type )
		{
		}
	};

	class Namespace
	{
	public:
		enum class Error {
			kSucces = 0,
			kError	= -1,
		};

	private:
		std::string				       m_name;
		std::unordered_map< std::string, Dependence >  m_dependences;
		std::unordered_map< std::string, ConfigValue > m_config_values;
		std::string				       __error;

		static bool isValidNamespace( const std::string& str )
		{
			return ctre::match< R"([a-zA-Z_][a-zA-Z0-9_]*(::[a-zA-Z_][a-zA-Z0-9_]*)*)" >( str );
		}

	public:
		explicit Namespace( std::string&& name )
		{
			this->name( std::move( name ) );
		}
		explicit Namespace( const std::string& name )
		{
			this->name( name );
		}

		auto& name( )
		{
			return m_name;
		}

		Error name( const std::string& name )
		{
			if ( isValidNamespace( name ) ) {
				m_name = name;
				return Error::kSucces;
			}
			return Error::kError;
		}

		const std::string& error( )
		{
			return __error;
		}

		static std::function< bool( const std::string& ) > f_validator( )
		{
			return isValidNamespace;
		}
	};

} // namespace Logic
