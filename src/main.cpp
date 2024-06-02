#include <fui/fui.h>
#include <filesystem>
#include <thread>
#include <unordered_map>
#include <vector>

#include "Logic/config.hpp"
#include "Logic/worker.hpp"

namespace fui = fungal;
using fui::ui;

GLobalState g_state{ "", std::filesystem::current_path( ).string( ), "", std::filesystem::current_path( ).string( ), "", Config( "global" ) };

int main( )
{
	ui.init( );

	Worker worker;

	{
		auto* page = new fui::page( "menu[start]", "Menu" );

		{ // Main Menu
			*page << new fui::button( "btn[new project]", "New Project", [ & ]( fui::control* c ) {
				worker.state( Worker::ProgramState::kNewProject );
				worker.work = [ & ]( ) -> std::optional< std::string > {
					std::this_thread::sleep_for( std::chrono::seconds( 1 ) );
					return "menu[new project]";
				};
				worker.flag( Worker::Interrupt::WorkerGet );
				worker.wait( );
				ui.display( worker.next_page( ) );
			} );
			*page << new fui::button( "btn[open project]", "Open Project", [ & ]( fui::control* c ) {
				worker.state( Worker::ProgramState::kOpenProject );
			} );
			*page << new fui::button( "btn[exit]", "Exit", [ & ]( fui::control* c ) {
				worker.flag( Worker::Interrupt::kStopWorker );
				ui.exit( );
			} );
		}

		ui << page;
		ui.display( page );
	}

	{	  // Create Project
		auto* page = new fui::page( "menu[new project]", "New Project" );

		{ // Project Name
			*page << new fui::textfield( "txt[project name]", "Project name" );
		}

		{ // Project Location
			*page << new fui::textfield( "txt[project dir]", "Project Location" );
		}

		{ // Create
			*page << new fui::button( "btn[create]", "Create", [ & ]( fui::control* c ) {
				worker.flag( Worker::Interrupt::WorkerGet );
				worker.wait( );
				ui.display( worker.next_page( ) );
			} );
		}

		{ // Back
			*page << new fui::button( "btn[back]", "Back", [ & ]( fui::control* c ) { ui.display( worker.next_page( ) ); } );
		}
		ui << page;
	}

	ui.exec( );
	worker.join( );
}
