/** \file md5.h
 * Displayer class interface and classic standard displayers
 *
 * $Id: md5.h,v 1.1 2004/07/28 07:55:51 besson Exp $
 */

/* Copyright, 2000, 2001, 2002,2003,2004 Nevrax Ltd.
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

#ifndef NL_MD5_H
#define NL_MD5_H

#include "nel/misc/types_nl.h"

#include <string>

namespace NLMISC
{

// ****************************************************************************
/**
 * MD5 High level routines
 * Largely inspired from the RSA Data Security works
 * \author Matthieu Besson
 * \author Nevrax France
 * \date July 2004
 */

inline bool operator <(const struct CHashKeyMD5 &a,const struct CHashKeyMD5 &b)
{
	return a < b;
}

// This function get a filename (it works with big files) and returns his MD5 hash key
CHashKeyMD5 getMD5(const std::string &filename);

// This function get a buffer with size and returns his MD5 hash key
CHashKeyMD5 getMD5(const uint8 *buffer, uint32 size);


// ****************************************************************************
/**
 * MD5 Low level routines
 * Largely inspired from the RSA Data Security works
 * \author Matthieu Besson
 * \author Nevrax France
 * \date July 2004
 */
/* Copyright (C) 1991-2, RSA Data Security, Inc. Created 1991. All
   rights reserved.

   License to copy and use this software is granted provided that it
   is identified as the "RSA Data Security, Inc. MD5 Message-Digest
   Algorithm" in all material mentioning or referencing this software
   or this function.

   License is also granted to make and use derivative works provided
   that such works are identified as "derived from the RSA Data
   Security, Inc. MD5 Message-Digest Algorithm" in all material
   mentioning or referencing the derived work.

   RSA Data Security, Inc. makes no representations concerning either
   the merchantability of this software or the suitability of this
   software for any particular purpose. It is provided "as is"
   without express or implied warranty of any kind.

   These notices must be retained in any copies of any part of this
   documentation and/or software.
   */

// ****************************************************************************
struct CHashKeyMD5
{
	uint8 Data[16];

	void clear();
	std::string toString() const;
	bool fromString(const std::string &in);
	bool operator!=(const CHashKeyMD5 &in) const;
	bool operator<(const CHashKeyMD5 &in) const;
};

// ****************************************************************************
class CMD5Context
{

public:

	void init();
	void update (const uint8 *pBufIn, uint32 nBufLength);
	void final (CHashKeyMD5 &out);

private:

	uint32	State[4];	// state (ABCD)
	uint32	Count[2];	// number of bits, modulo 2^64 (lsb first)
	uint8	Buffer[64]; // input buffer

	static uint8 Padding[64];

private:

	void transform (uint32 state[4], const uint8 block[64]);
	void encode (uint8 *output, const uint32 *input, uint len);
	void decode (uint32 *output, const uint8 *input, uint len);

};

}; // namespace NLMISC

#endif // NL_MD5_H

/* End of md5.h */
