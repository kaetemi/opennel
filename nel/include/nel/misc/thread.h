/** \file thread.h
 * thread interface
 *
 * $Id: thread.h,v 1.11 2001/02/13 18:25:57 corvazier Exp $
 */

/* Copyright, 2000 Nevrax Ltd.
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

#ifndef NL_THREAD_H
#define NL_THREAD_H

#include "nel/misc/types_nl.h"
#include "nel/misc/common.h"


namespace NLMISC {


/**
 * Thread callback interface.
 * When a thread is created, it will call run() in its attached IRunnable interface.
 *
 *\code

	#include "nel/misc/thread.h"

	class HelloLoopThread : public IRunnable
	{
		void run ()
		{
			while(true)	printf("Hello World\n");
		}

	};

	IThread *thread = IThread::create (new HelloLoopThread);
	thread->start ();

 *\endcode
 *
 *
 *
 *
 * \author Vianney Lecroart
 * \author Nevrax France
 * \date 2000
 */
class IRunnable
{
public:
	// Called when a thread is run.
	virtual void run()=0;
	virtual ~IRunnable()
	{
	}
};

/**
 * Thread base interface, must be implemented for all OS
 * \author Vianney Lecroart
 * \author Nevrax France
 * \date 2000
 */
class IThread
{
public:

	/// implemented in the derived class
	static IThread *create(IRunnable *runnable);

	virtual ~IThread () { }
	
	// Starts the thread.
	virtual void start()=0;

	// Terminate the thread. (use with caution under win98)
	virtual void terminate()=0;

	// In the calling program, wait until the specified thread has existed (use with caution under win98)
	virtual void wait()=0;

	// Sleep the thread
	virtual void sleep(sint ms = 0) =0;
};


struct EThread : public Exception
{
	EThread (const char* message) : Exception (message) {};
};


} // NLMISC


#endif // NL_THREAD_H

/* End of thread.h */
