/** \file p_thread.cpp
 * <File description>
 *
 * $Id: p_thread.cpp,v 1.4 2001/02/14 13:47:24 cado Exp $
 */

/* Copyright, 2001 Nevrax Ltd.
 *
 * This file is part of NEVRAX NEL.
 * NEVRAX NEL is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.

 * NEVRAX NEL is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with NEVRAX NEL; see the file COPYING. If not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA.
 */


#include "nel/misc/types_nl.h"

#ifdef NL_OS_UNIX

#include "nel/misc/p_thread.h"


namespace NLMISC {



/*
 * The IThread static creator
 */
IThread *IThread::create( IRunnable *runnable )
{
	return new CPThread( runnable );
}


/*
 * Thread beginning
 */
static void *ProxyFunc( void *arg )
{
	CPThread *parent = (CPThread*)arg;

	// Allow to terminate the thread without cancellation point
	pthread_setcanceltype( PTHREAD_CANCEL_ASYNCHRONOUS, NULL );

	// Run the code of the thread
	parent->Runnable->run();

	return NULL;
}



/*
 * Constructor
 */
CPThread::CPThread( IRunnable *runnable ) : Runnable( runnable ), _Started( false )
{}


/*
 * Destructor
 */
CPThread::~CPThread()
{
	pthread_detach( _ThreadHandle ); // free allocated resources after termination
	if ( _Started )
	{
		terminate();
	}
}


/*
 * start
 */
void CPThread::start()
{
	if ( pthread_create( &_ThreadHandle, NULL, ProxyFunc, this ) != 0 )
	{
		throw EThread( "Cannot start new thread" );
	}
	_Started = true;
}


/*
 * terminate
 */
void CPThread::terminate()
{
	pthread_cancel( _ThreadHandle );
	_Started = false;
}


/*
 * wait
 */
void CPThread::wait ()
{
	if ( _Started )
	{
		if ( pthread_join( _ThreadHandle, NULL ) != 0 )
		{
			throw EThread( "Cannot join with thread" );
		}
		_Started = false;
	}
}


} // NLMISC

#endif // NL_OS_UNIX
