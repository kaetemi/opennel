/** \file string_id_array.h
 * Container that provides associations between string and integer
 *
 * $Id: string_id_array.h,v 1.16 2005/01/31 15:11:22 cado Exp $
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

#ifndef NL_STRING_ID_ARRAY_H
#define NL_STRING_ID_ARRAY_H

#include "nel/misc/types_nl.h"

#include <cmath>
#include <string>
#include <vector>
#include <set>
#include <algorithm>

#include "nel/misc/debug.h"


namespace NLMISC {


/**
 * The goal of this class is to associate number and string. It is used in the
 * CCallbackNetBase for message id<->string associations.
 * \author Vianney Lecroart
 * \author Nevrax France
 * \date 2001
 */
class CStringIdArray
{
public:

	typedef sint16 TStringId;

	CStringIdArray () : _IgnoreAllUnknownId(false) { }

	/// Adds a string in the string in the array
	void addString(const std::string &str, TStringId id)
	{
		nlassert (id >= 0 && id < pow(2.0, (sint)sizeof (TStringId)*8));
		nlassert (!str.empty());

		if (id >= (sint32) _StringArray.size())
			_StringArray.resize(id+1);

		_StringArray[id] = str;

		if (!_IgnoreAllUnknownId)
		{
			_AskedStringArray.erase (str);
			_NeedToAskStringArray.erase (str);
		}
	}

	/// Adds a string in the string at the end of the array
	void addString(const std::string &str)
	{
		nlassert (_StringArray.size () < pow(2.0, (sint)sizeof (TStringId)*8));
		nlassert (!str.empty());

		// add at the end
		addString (str, _StringArray.size ());
	}

	/** Returns the id associated to string str. If the id is not found, the string will be added in
	 * _NeedToAskStringArray if ignoreAllUnknownId is false and IgnoreIfUnknown is false too.
	 */
	TStringId getId (const std::string &str, bool IgnoreIfUnknown = false)
	{
		nlassert (!str.empty());

		// sorry for this bullshit but it's the simplest way ;)
		if (this == NULL) return -1;

		for (TStringId i = 0; i < (TStringId) _StringArray.size(); i++)
		{
			if (_StringArray[i] == str)
				return i;
		}

		if (!_IgnoreAllUnknownId && !IgnoreIfUnknown)
		{
			// the string is not found, add it to the _AskedStringArray if necessary
			if (_NeedToAskStringArray.find (str) == _NeedToAskStringArray.end ())
			{
				if (_AskedStringArray.find (str) == _AskedStringArray.end ())
				{
					//nldebug ("String '%s' not found, add it to _NeedToAskStringArray", str.c_str ());	
					_NeedToAskStringArray.insert (str);
				}
				else
				{
					//nldebug ("Found '%s' in the _AskedStringArray", str.c_str ());	
				}
			}
			else
			{
				//nldebug ("Found '%s' in the _NeedToAskStringArray", str.c_str ());	
			}
		}
		else
		{
			//nldebug ("Ignoring unknown association ('%s')", str.c_str ());	
		}

		return -1;
	}

	/// Returns the string associated to this id
	std::string getString (TStringId id) const
	{
		// sorry for this bullshit but it's the simplest way ;)
		if (this == NULL) return "<NoSIDA>";

		nlassert (id >= 0 && id < (TStringId)_StringArray.size());

		return _StringArray[id];
	}

	/// Set the size of the _StringArray
	void resize (TStringId size)
	{
		_StringArray.resize (size);
	}

	/// Returns the size of the _StringArray
	TStringId size () const
	{
		return _StringArray.size ();
	}

	/// Returns all string in the _NeedToAskStringArray
	const std::set<std::string> &getNeedToAskedStringArray () const
	{
		return _NeedToAskStringArray;
	}

	/// Returns all string in the _AskedStringArray
	const std::set<std::string> &getAskedStringArray () const
	{
		return _AskedStringArray;
	}

	/// Moves string from _NeedToAskStringArray to _AskedStringArray
	void moveNeedToAskToAskedStringArray ()
	{
		if (!_IgnoreAllUnknownId)
		{
			_AskedStringArray.insert (_NeedToAskStringArray.begin(), _NeedToAskStringArray.end());
			_NeedToAskStringArray.clear ();
		}
	}

	/// If set to true, when we ask a string with no id, we don't put it in the _NeedToAskStringArray array
	void ignoreAllUnknownId (bool b) { _IgnoreAllUnknownId = b; }

	/// Clears the string id array
	void clear ()
	{
		_StringArray.clear ();
		_NeedToAskStringArray.clear ();
		_AskedStringArray.clear ();
	}

	/// Displays all association of the array in a C style (useful to copy/paste in your C code)
	void display ()
	{
		nlinfo ("static const char *OtherSideAssociations[] = {");
		for (uint i = 0; i < _StringArray.size(); i++)
		{
			nlinfo(" \"%s\",", _StringArray[i].c_str());
		}
		nlinfo ("};");
	}

private:

	bool _IgnoreAllUnknownId;

	std::vector<std::string>	_StringArray;

	std::set<std::string>	_NeedToAskStringArray;

	std::set<std::string>	_AskedStringArray;
};


} // NLMISC


#endif // NL_STRING_ID_ARRAY_H

/* End of string_id_array.h */
