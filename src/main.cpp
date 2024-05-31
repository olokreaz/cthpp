#include <curses.h>
#include <string>
#include <thread>

int main( )
{
	initscr( );		// Initialize the library
	cbreak( );		// Line buffering disabled
	keypad( stdscr, TRUE ); // We get F1, F2 etc..
	noecho( );		// Don't echo() while we do getch

	// Enable mouse events
	mousemask( ALL_MOUSE_EVENTS, NULL );

	int height, width, start_y, start_x;
	height	= 10;
	width	= 40;
	start_y = ( LINES - height ) / 2;			 // Calculating for a center placement of the window
	start_x = ( COLS - width ) / 2;				 // Calculating for a center placement of the window

	WINDOW* win = newwin( height, width, start_y, start_x ); // Create a new window
	box( win, 0, 0 );					 // Set the window borders

	int ch = 0;

	std::thread t1( [ & ]( ) {
		MEVENT event;
		while ( ch != 'q' ) {
			ch = getch( );				 // Get the user input
			if ( ch == KEY_MOUSE ) {
				if ( auto x = getmouse( ); ) {
					// Print the mouse position
					mvwprintw( win, 1, 1, "Mouse at: %d, %d", event.x, event.y );
					wrefresh( win );
				}
			}
		}
	} );
	while ( ch != 'q' ) {
		std::this_thread::sleep_for( std::chrono::milliseconds( 10 ) ); // Sleep for 10 milliseconds
		std::string title	  = "Program Title";
		int	    title_start_x = ( width - title.size( ) ) / 2;	// Calculate the start position of the title
		mvwprintw( win, 0, title_start_x, title.c_str( ) );		// Print the title on the window
		wrefresh( win );						// Refresh the window
	}

	t1.join( );
	endwin( );								// End curses mode

	return 0;
}
