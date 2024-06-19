#include "olokreaz/FileGeneration/HeaderGenerator.hpp"

int main( int argc, char** argv )
{

	HeaderGenerator hg( "Copyright @olokreaz", "config.h" );

	hg.addNamespace( "config" );
	hg.addVariable( "config", "auto", "name", "\"LoFrend\"" );
	hg.addNamespace( "config::client" );
	hg.addNamespace( "config::client::network" );
	hg.addVariable( "config::client::network", "auto", "author", "\"@olokreaz\"" );
	hg.addVariable( "config::client::network", "auto", "date", "\"19.06.2024\"" );

	hg.generate( );

	return 0;
}
