/** \file tds.h
 * Thread dependant storage class
 *
 * $Id$
 */

/* Copyright, 2000-2002 Nevrax Ltd.
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

#ifndef NL_TDS_H
#define NL_TDS_H

#include "types_nl.h"


namespace NLMISC 
{


/**
 * Thread dependant storage class
 *
 * This class provides a thread specific (void*).
 * It is initialized at NULL.
 *
 * \author Cyril 'Hulud' Corvazier
 * \author Nevrax France
 * \date 2002
 */
class CTDS
{
public:

	/// Constructor. The pointer is initialized with NULL.
	CTDS ();

	/// Destructor
	~CTDS ();

	/// Get the thread specific pointer
	void	*getPointer () const;

	/// Set the thread specific pointer
	void	setPointer (void* pointer);

private:
#ifdef NL_OS_WINDOWS
	uint32			_Handle;
#else // NL_OS_WINDOWS
	pthread_key_t	_Key;
#endif // NL_OS_WINDOWS

};


} // NLMISC


#endif // NL_TDS_H

/* End of tds.h */
