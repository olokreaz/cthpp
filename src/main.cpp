#include <array>				  // for array
#include <atomic>				  // for atomic
#include <chrono>				  // for operator""s, chrono_literals
#include <cmath>				  // for sin
#include <functional>				  // for ref, reference_wrapper, function
#include <memory>				  // for allocator, shared_ptr, __shared_ptr_access
#include <stddef.h>				  // for size_t
#include <string>				  // for string, basic_string, char_traits, operator+, to_string
#include <thread>				  // for sleep_for, thread
#include <utility>				  // for move
#include <vector>				  // for vector

#include "ftxui/component/component.hpp"	  // for Checkbox, Renderer, Horizontal, Vertical, Input, Menu, Radiobox, ResizableSplitLeft, Tab
#include "ftxui/component/component_base.hpp"	  // for ComponentBase, Component
#include "ftxui/component/component_options.hpp"  // for MenuOption, InputOption
#include "ftxui/component/event.hpp"		  // for Event, Event::Custom
#include "ftxui/component/screen_interactive.hpp" // for Component, ScreenInteractive
#include "ftxui/dom/elements.hpp" // for text, color, operator|, bgcolor, filler, Element, vbox, size, hbox, separator, flex, window, graph, EQUAL, paragraph, WIDTH, hcenter, Elements, bold, vscroll_indicator, HEIGHT, flexbox, hflow, border, frame, flex_grow, gauge, paragraphAlignCenter, paragraphAlignJustify, paragraphAlignLeft, paragraphAlignRight, dim, spinner, LESS_THAN, center, yframe, GREATER_THAN
#include "ftxui/dom/flexbox_config.hpp" // for FlexboxConfig
#include "ftxui/screen/color.hpp" // for Color, Color::BlueLight, Color::RedLight, Color::Black, Color::Blue, Color::Cyan, Color::CyanLight, Color::GrayDark, Color::GrayLight, Color::Green, Color::GreenLight, Color::Magenta, Color::MagentaLight, Color::Red, Color::White, Color::Yellow, Color::YellowLight, Color::Default, Color::Palette256, ftxui
#include "ftxui/screen/color_info.hpp" // for ColorInfo
#include "ftxui/screen/terminal.hpp"   // for Size, Dimensions

#include <algorithm>
#include <ranges>
#include <unordered_map>

#include <ctre.hpp>

using namespace ftxui;

bool isValid( const std::string_view str )
{
	using namespace ctre::literals;
	return R"(\w+)"_ctre.match( str );
}

enum class Ev : uint8_t {
	Exit = -1,
	None = 0,
};

struct ObjState;
struct EventObj {
	Ev								      ev;
	std::function< void( ObjState ) >				      fn;
	std::function< void( class UI* = nullptr, class Worker* = nullptr ) > cb;
};

class UI
{
	friend class Worker;
	std::unordered_map< int, std::string > m_id_to_title;
	std::unordered_map< std::string, int > m_title_to_id;

	std::string	 m_title;
	int		 m_id;
	std::atomic_bool m_bRefresh = true;

	class Worker* pw = nullptr;

	ScreenInteractive screen;

	auto MENU_BTN_STYLE( )
	{
		auto opt      = ButtonOption::Simple( );
		opt.transform = []( const EntryState& st ) {
			auto el = text( st.label );
			if ( st.focused ) el = el | bold | color( Color::BlueLight );
			return el | center;
		};
		return opt;
	}

	void refresh( )
	{
		if ( m_bRefresh ) {
			using namespace std::chrono_literals;
			std::this_thread::sleep_for( 0.001ms );

			if ( m_id >= m_id_to_title.size( ) ) m_id = 0;
			if ( m_id < 0 ) m_id = m_id_to_title.size( ) - 1;
			m_title = m_id_to_title[ m_id ];
		}
	}

public:
	UI( ) : screen{ ScreenInteractive::Fullscreen( ) }
	{

		m_id_to_title = {
				{ 0, "Welcome" },
				{ 1, "New Project" },
				{ 1, "Open project" },
		};
		m_title_to_id = {
				{ "welcome", 0 },
				{ "new project", 1 },
				{ "open project", 1 },
		};
	}

	void start( )
	{
	}

	void to_page( std::string title )
	{
		std::transform( title.begin( ), title.end( ), title.begin( ), ::tolower );
		m_id = m_title_to_id[ title ];
	}
};

class Worker
{
	std::thread* m_pThread = nullptr;
	UI&	     m_ui;

	std::queue< Ev > m_events;

	void update( )
	{
	}

public:
	explicit Worker( UI& ui ) : m_ui{ ui }
	{
		m_ui.pw = this;
	}

	void start( )
	{
	}

	void push_event( enum class Ev e )
	{
	}
};

int main( )
{

	auto	    page_selector = 0;
	std::string title	  = "startup";

	auto btn_create_project = Button( "create new project", [ & ] { page_selector = title_page_to_id[ "new project" ]; }, MENU_BTN_STYLE( ) );

	auto btn_open_project = Button( "open project", [ & ] { page_selector = title_page_to_id[ "open project" ]; }, MENU_BTN_STYLE( ) );

	auto btn_exit = Button( "exit", [ & ] { screen.Exit( ); }, MENU_BTN_STYLE( ) );

	auto welocme_page = Container::Vertical( { btn_create_project, btn_open_project, btn_exit } );

	auto welcome_page_renderer = Renderer( welocme_page, [ & ] {
		return vbox( {
				       btn_create_project->Render( ) | center,
				       btn_open_project->Render( ) | center,
				       btn_exit->Render( ) | center,
		       } ) |
		       center;
	} );

	std::string project_name = "";
	bool	    bReTitle	 = false;

	auto option_input_project_name	      = InputOption::Spacious( );
	option_input_project_name.placeholder = "Project name";
	option_input_project_name.multiline   = false;
	option_input_project_name.password    = false;
	option_input_project_name.content     = &project_name;
	option_input_project_name.on_change   = [ & ] {
		  if ( project_name.size( ) > 256 ) project_name = project_name.substr( 0, 256 );
		  if ( !isValid( project_name ) ) project_name.pop_back( );
		  bReTitle = true;
	};

	auto inp_project_name	     = Input( option_input_project_name );
	auto btn_create_project_page = Button(
			"create projcet",
			[ & ] {
				if ( project_name.empty( ) ) return;
				/* alert "project name is empty!" */
				++page_selector;
			},
			MENU_BTN_STYLE( ) );

	auto btn_back = Button( "< back", [ & ] { --page_selector; }, MENU_BTN_STYLE( ) );

	auto create_project_page	  = Container::Vertical( { inp_project_name, btn_create_project_page, btn_back } );
	auto create_project_page_renderer = Renderer( create_project_page, [ & ] {
		return vbox( {
				       inp_project_name->Render( ) | size( WIDTH, LESS_THAN, 260 ) | size( WIDTH, GREATER_THAN, 10 ) | center,
				       btn_create_project_page->Render( ) | center,
				       btn_back->Render( ) | center,
		       } ) |
		       center;
	} );

	auto page_content = Container::Tab( { welcome_page_renderer, create_project_page_renderer }, &page_selector );

	auto main_container = Container::Vertical( { page_content } );

	auto main_renderer = Renderer( main_container, [ & ] {
		return vbox( { text( "oloproxx configurator" ) | bold | hcenter,
			       window( text( title ) | color( Color::Gold1 ) | center, page_content->Render( ) ) | flex } );
	} );

	std::atomic refresh_ui_continue = true;
	std::thread refresh_ui( [ & ] {
		while ( refresh_ui_continue ) {
			using namespace std::chrono_literals;
			std::this_thread::sleep_for( 0.001ms );

			if ( page_selector >= id_page_to_title.size( ) ) page_selector = 0;
			title = id_page_to_title[ page_selector ];

			if ( bReTitle && !project_name.empty( ) ) title = project_name;

			screen.Post( Event::Custom );
		}
	} );

	screen.Loop( main_renderer );
	refresh_ui_continue = false;
	refresh_ui.join( );

	return 0;
}
