/** \file stream_inline.h
 * This File Declares inline for stream.h CRegistry and CBaseStream 
 *
 * \todo yoyo: passer les serial en virtual, et optimiser l'ecriture en NeedSwap. => implementer les 
 * serial virtual dans les classes dérivées. => pour optimiser, fodra pitet virer setInOut(), utile pour CMessage
 * Pkoi? : pour optimiser la lecture/ecriture (plus de if du tout). Plus rapide pour olivier de faire des copies
 * de messages (brut) que de se taper un if dans le CMessage.
 *
 * $Id: stream_inline.h,v 1.19 2001/05/11 14:03:36 lecroart Exp $
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

#ifndef NL_STREAM_INLINE_H
#define NL_STREAM_INLINE_H

#include <nel/misc/debug.h>


namespace	NLMISC
{


// ======================================================================================================
// ======================================================================================================
// IBasicStream Inline Implementation.
// ======================================================================================================
// ======================================================================================================


// ======================================================================================================
// ======================================================================================================
// ======================================================================================================

// ======================================================================================================
inline	IStream::IStream(bool inputStream, bool needSwap)
{
	_InputStream= inputStream;
#ifdef NL_LITTLE_ENDIAN
	_NeedSwap=false;
#else
	_NeedSwap= needSwap;
#endif
}


// ======================================================================================================
inline	bool		IStream::isReading() const
{
	return _InputStream;
}


// ======================================================================================================
// ======================================================================================================
// ======================================================================================================

// ======================================================================================================
inline	void		IStream::serial(uint8 &b) 
{
	serialBuffer((uint8 *)&b, 1);
}

// ======================================================================================================
inline	void		IStream::serial(sint8 &b) 
{
	serialBuffer((uint8 *)&b, 1);
}

// ======================================================================================================
inline	void		IStream::serial(uint16 &b) 
{
	if(!_NeedSwap)
	{
		serialBuffer((uint8 *)&b, 2);
	}
	else
	{
		uint16	v;
		if(isReading())
		{
			serialBuffer((uint8 *)&v, 2);
			NLMISC_BSWAP16(v);
			b=v;
		}
		else
		{
			v=b;
			NLMISC_BSWAP16(v);
			serialBuffer((uint8 *)&v, 2);
		}
	}
}

// ======================================================================================================
inline	void		IStream::serial(sint16 &b) 
{
	if(!_NeedSwap)
	{
		serialBuffer((uint8 *)&b, 2);
	}
	else
	{
		uint16	v;
		if(isReading())
		{
			serialBuffer((uint8 *)&v, 2);
			NLMISC_BSWAP16(v);
			b=v;
		}
		else
		{
			v=b;
			NLMISC_BSWAP16(v);
			serialBuffer((uint8 *)&v, 2);
		}
	}
}

// ======================================================================================================
inline	void		IStream::serial(uint32 &b) 
{
	if(!_NeedSwap)
	{
		serialBuffer((uint8 *)&b, 4);
	}
	else
	{
		uint32	v;
		if(isReading())
		{
			serialBuffer((uint8 *)&v, 4);
			NLMISC_BSWAP32(v);
			b=v;
		}
		else
		{
			v=b;
			NLMISC_BSWAP32(v);
			serialBuffer((uint8 *)&v, 4);
		}
	}
}

// ======================================================================================================
inline	void		IStream::serial(sint32 &b) 
{
	if(!_NeedSwap)
	{
		serialBuffer((uint8 *)&b, 4);
	}
	else
	{
		uint32	v;
		if(isReading())
		{
			serialBuffer((uint8 *)&v, 4);
			NLMISC_BSWAP32(v);
			b=v;
		}
		else
		{
			v=b;
			NLMISC_BSWAP32(v);
			serialBuffer((uint8 *)&v, 4);
		}
	}
}

// ======================================================================================================
inline	void		IStream::serial(uint64 &b) 
{
	if(!_NeedSwap)
	{
		serialBuffer((uint8 *)&b, 8);
	}
	else
	{
		uint64	v;
		if(isReading())
		{
			serialBuffer((uint8 *)&v, 8);
			NLMISC_BSWAP64(v);
			b=v;
		}
		else
		{
			v=b;
			NLMISC_BSWAP64(v);
			serialBuffer((uint8 *)&v, 8);
		}
	}
}

// ======================================================================================================
inline	void		IStream::serial(sint64 &b) 
{
	if(!_NeedSwap)
	{
		serialBuffer((uint8 *)&b, 8);
	}
	else
	{
		uint64	v;
		if(isReading())
		{
			serialBuffer((uint8 *)&v, 8);
			NLMISC_BSWAP64(v);
			b=v;
		}
		else
		{
			v=b;
			NLMISC_BSWAP64(v);
			serialBuffer((uint8 *)&v, 8);
		}
	}
}

// ======================================================================================================
inline	void		IStream::serial(float &b) 
{
	if(!_NeedSwap)
	{
		serialBuffer((uint8 *)&b, 4);
	}
	else
	{
		uint32	v;
		if(isReading())
		{
			serialBuffer((uint8 *)&v, 4);
			NLMISC_BSWAP32(v);
			b=*((float*)&v);
		}
		else
		{
			v=*((uint32*)&b);
			NLMISC_BSWAP32(v);
			serialBuffer((uint8 *)&v, 4);
		}
	}
}

// ======================================================================================================
inline	void		IStream::serial(double &b) 
{
	if(!_NeedSwap)
	{
		serialBuffer((uint8 *)&b, 8);
	}
	else
	{
		uint64	v;
		if(isReading())
		{
			serialBuffer((uint8 *)&v, 8);
			NLMISC_BSWAP64(v);
			b=*((double*)&v);
		}
		else
		{
			v=*((uint64*)&b);
			NLMISC_BSWAP64(v);
			serialBuffer((uint8 *)&v, 8);
		}
	}
}

// ======================================================================================================
inline	void		IStream::serial(bool &b) 
{
	serialBit(b);
}

#ifndef NL_OS_CYGWIN
// ======================================================================================================
inline	void		IStream::serial(char &b) 
{
	serialBuffer((uint8 *)&b, 1);
}
#endif

// ======================================================================================================
inline	void		IStream::serial(std::string &b) 
{
	sint32	len=0;
	// Read/Write the length.
	if(isReading())
	{
		serial(len);
		nlassert( len<1000000 ); // limiting string size
		b.resize(len);
	}
	else
	{
		len= b.size();
		serial(len);
	}
	// Read/Write the string.
	for(sint i=0;i<len;i++)
		serial(b[i]);
}


// ======================================================================================================
inline	void		IStream::serial(ucstring &b) 
{
	sint32	len=0;
	// Read/Write the length.
	if(isReading())
	{
		serial(len);
		b.resize(len);
	}
	else
	{
		len= b.size();
		serial(len);
	}
	// Read/Write the string.
	for(sint i=0;i<len;i++)
		serial(b[i]);
}



// ======================================================================================================
inline uint8			IStream::serialBitField8(uint8  bf)
{
	serial(bf);
	return bf;
}
// ======================================================================================================
inline uint16			IStream::serialBitField16(uint16  bf)
{
	serial(bf);
	return bf;
}
// ======================================================================================================
inline uint32			IStream::serialBitField32(uint32  bf)
{
	serial(bf);
	return bf;
}


}


#endif // NL_STREAM_INLINE_H

/* End of stream_inline.h */
