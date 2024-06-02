#pragma once
#include "config.hpp"

#include <atomic>
#include <functional>
#include <queue>
#include <thread>

class Worker
{
public:
	enum class Interrupt {
		kNone = 0,
		kStopWorker,
		UiGet,
		WorkerGet,
	};

	enum class ProgramState {
		kMainMenu,
		kNewProject,
		kOpenProject,
		kProject,
	};

private:
	std::thread*		 m_thread{ nullptr };
	std::atomic< Interrupt > m_interrupt;

	std::string  m_next_page;
	ProgramState m_program_state{ };

public:
	std::function< std::optional< std::string >( ) > work;

	void start( )
	{
		if ( m_thread != nullptr ) return;
		m_thread = new std::thread( [ this ]( ) {
			while ( m_interrupt != Interrupt::kStopWorker ) {
				std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );
				if ( m_interrupt == Interrupt::WorkerGet ) {
					auto x = work( );
					if ( x ) m_next_page = *x;
					m_interrupt = Interrupt::UiGet;
				}
			}
		} );
	}

	void wait( ) const
	{
		while ( m_interrupt != Interrupt::UiGet ) std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );
	}

	const std::string& next_page( )
	{
		return m_next_page;
	}

	void flag( Interrupt interrupt )
	{
		m_interrupt = interrupt;
	}

	auto flag( ) const
	{
		return m_interrupt.load( );
	}

	void state( ProgramState state )
	{
		m_program_state = ProgramState::kNewProject;
	}

	auto state( ) const
	{
		return m_program_state;
	}

	void join( )
	{
		m_interrupt = Interrupt::kStopWorker;
		if ( m_thread == nullptr ) return;
		m_thread->join( );
		delete m_thread;
		m_thread = nullptr;
	}
};
