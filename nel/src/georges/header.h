/** \file header.h
 * Georges header file class
 *
 * $Id: header.h,v 1.4 2005/01/17 16:39:42 lecroart Exp $
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

#ifndef NL_HEADER_H
#define NL_HEADER_H

#include	"nel/misc/types_nl.h"

namespace NLGEORGES
{

class CFileHeader
{
public:
	/// Default constructor
	CFileHeader ();

	// Form states
	enum TState
	{
		Modified =0,
		Checked,
		StateCount,
	};

	/// Add a log
	void				addLog (const char *log);

	/// Set the comment
	void				setComments (const char *comments);

	/// Major version number
	uint32				MajorVersion;

	/// Minor version number
	uint32				MinorVersion;

	/// State of the form
	TState				State;

	/// CVS Revision string
	std::string			Revision;

	/// Comments of the form
	std::string			Comments;

	/// Log of the form
	std::string			Log;

	/// ** IO functions
	void				read (xmlNodePtr root);
	void				write (xmlNodePtr node, bool georges4CVS) const;

	// Get state string
	static const char	*getStateString (TState state);

	// Error handling
	void				warning (bool exception, const char *function, const char *format, ... ) const;
};

} // NLGEORGES

#endif // NL_HEADER_H
