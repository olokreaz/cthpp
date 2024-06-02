#include "config.hpp"
#include <ctre.hpp>

Logic::ConfigValue::ConfigType Config::_parseString( const std::string& str ) const
{
	if ( ctre::match< "0x([0-9a-fA-F]+)" >( str ) ) {
		uint64_t num = std::stoull( str, nullptr, 16 );
		if ( num <= 0xFF ) return Logic::ConfigValue::ConfigType::kU8;
		if ( num <= 0xFFFF ) return Logic::ConfigValue::ConfigType::kU16;
		if ( num <= 0xFFFFFFFF ) return Logic::ConfigValue::ConfigType::kU32;
		return Logic::ConfigValue::ConfigType::kU64;
	}

	if ( ctre::match< "0([0-8]+)" >( str ) ) {
		uint64_t num = std::stoull( str, nullptr, 8 );
		if ( num <= 0xFF ) return Logic::ConfigValue::ConfigType::kU8;
		if ( num <= 0xFFFF ) return Logic::ConfigValue::ConfigType::kU16;
		if ( num <= 0xFFFFFFFF ) return Logic::ConfigValue::ConfigType::kU32;
		return Logic::ConfigValue::ConfigType::kU64;
	}

	if ( ctre::match< R"((|\+|-)(\d+\.(\d+|)|(\d+|)\.\d+))" >( str ) ) return Logic::ConfigValue::ConfigType::kDouble;
	if ( ctre::match< R"((|\+|-)(\d+\.(\d+|)|(\d+|)\.\d+)f)" >( str ) ) return Logic::ConfigValue::ConfigType::kFloat;

	if ( ctre::match< R"(\d+)" >( str ) ) {
		uint64_t num = std::stoull( str );
		if ( num <= 0xFF ) return Logic::ConfigValue::ConfigType::kU8;
		if ( num <= 0xFFFF ) return Logic::ConfigValue::ConfigType::kU16;
		if ( num <= 0xFFFFFFFF ) return Logic::ConfigValue::ConfigType::kU32;
		return Logic::ConfigValue::ConfigType::kU64;
	}

	if ( ctre::match< R"(-\d+)" >( str ) ) {
		int64_t num = std::stoll( str );
		if ( num >= -128 && num <= -1 ) return Logic::ConfigValue::ConfigType::kI8;
		if ( num >= -32768 && num <= -1 ) return Logic::ConfigValue::ConfigType::kI16;
		if ( num >= -2147483648 && num <= -1 ) return Logic::ConfigValue::ConfigType::kI32;
		return Logic::ConfigValue::ConfigType::kI64;
	}

	if ( ctre::match< R"((false|true))" >( str ) ) return Logic::ConfigValue::ConfigType::kBool;

	return Logic::ConfigValue::ConfigType::kString;
}
