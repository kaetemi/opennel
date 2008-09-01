/**
 * \file user_role.h
 * \brief TUserRole
 * \date 2005-10-03 05:15GMT
 * \author Nevrax Ltd. (Nevrax Ltd.)
 * TUserRole
 * 
 * $Id$
 */

/* 
 * Copyright (C) 2005  Nevrax Ltd. (Nevrax Ltd.)
 * 
 * This file is part of NEVRAX NeL Network Services.
 * NEVRAX NeL Network Services is free software: you can redistribute
 * it and/or modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation, either
 * version 2 of the License, or (at your option) any later version.
 * 
 * NEVRAX NeL Network Services is distributed in the hope that it will
 * be useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with NEVRAX NeL Network Services; if not, write to the Free
 * Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301 USA.
 */

#ifndef WS_USER_ROLE_H
#define WS_USER_ROLE_H
#include <nel/misc/types_nl.h>

// Project includes

// NeL includes
#include <nel/misc/stream.h>
#include <nel/misc/string_conversion.h>

// STL includes
#include <map>

namespace WS {

/**
 * \brief TUserRole
 * \date 2005-10-03 05:15GMT
 * \author Nevrax Ltd. (Nevrax Ltd.)
 * TUserRole
 */
struct TUserRole
{
	enum TValues
	{
		ur_player,
		ur_editor,
		ur_animator,
		/// the highest valid value in the enum
		last_enum_item = ur_animator,
		/// a value equal to the last enum item +1
		end_of_enum,

		invalid_val,
		
		/// Number of enumerated values
		nb_enum_items = 3
	};
	
	/// Index table to convert enum value to linear index table
	const std::map<TValues, uint32> &getIndexTable() const
	{
		static std::map<TValues, uint32> indexTable;
		static bool init = false;
		if (!init)
		{
			// fill the index table
			indexTable.insert(std::make_pair(ur_player, 0));
			indexTable.insert(std::make_pair(ur_editor, 1));
			indexTable.insert(std::make_pair(ur_animator, 2));
		
			init = true;
		}

		return indexTable;
	}
	

	static const NLMISC::CStringConversion<TValues> &getConversionTable()
	{
		NL_BEGIN_STRING_CONVERSION_TABLE(TValues)
			NL_STRING_CONVERSION_TABLE_ENTRY(ur_player)
			NL_STRING_CONVERSION_TABLE_ENTRY(ur_editor)
			NL_STRING_CONVERSION_TABLE_ENTRY(ur_animator)
			NL_STRING_CONVERSION_TABLE_ENTRY(invalid_val)
		};                                                                                             
		static NLMISC::CStringConversion<TValues>                                                                
		conversionTable(TValues_nl_string_conversion_table, sizeof(TValues_nl_string_conversion_table)   
		/ sizeof(TValues_nl_string_conversion_table[0]),  invalid_val);

		return conversionTable;
	}

	TValues	_Value;

public:
	TUserRole()
		: _Value(invalid_val)
	{
	}
	TUserRole(TValues value)
		: _Value(value)
	{
	}

	TUserRole(const std::string &str)
	{
		_Value = getConversionTable().fromString(str);
	}

	void serial(NLMISC::IStream &s)
	{
		s.serialEnum(_Value);
	}

	bool operator == (const TUserRole &other) const
	{
		return _Value == other._Value;
	}
	bool operator != (const TUserRole &other) const
	{
		return ! (_Value == other._Value);
	}
	bool operator < (const TUserRole &other) const
	{
		return _Value < other._Value;
	}

	bool operator <= (const TUserRole &other) const
	{
		return _Value <= other._Value;
	}

	bool operator > (const TUserRole &other) const
	{
		return !(_Value <= other._Value);
	}
	bool operator >= (const TUserRole &other) const
	{
		return !(_Value < other._Value);
	}

	const std::string &toString() const
	{
		return getConversionTable().toString(_Value);
	}
	static const std::string &toString(TValues value)
	{
		return getConversionTable().toString(value);
	}

	TValues getValue() const
	{
		return _Value;
	}

	// return true if the actual value of the enum is valid, otherwise false
	bool isValid()
	{
		if (_Value == invalid_val)
			return false;

		// not invalid, check other enum value
		return getConversionTable().isValid(_Value);
	}

	
	uint32 asIndex()
	{
		std::map<TValues, uint32>::const_iterator it(getIndexTable().find(_Value));
		nlassert(it != getIndexTable().end());
		return it->second;
	}
	
}; /* struct TUserRole */

} /* namespace WS */

#endif /* #ifndef WS_USER_ROLE_H */

/* end of file */
