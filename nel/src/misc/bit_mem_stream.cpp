/** \file bit_mem_stream.cpp
 * Bit-oriented memory stream
 *
 * $Id: bit_mem_stream.cpp,v 1.2 2001/10/08 14:03:33 cado Exp $
 */

/* Copyright, 2000, 2001 Nevrax Ltd.
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

#include "nel/misc/bit_mem_stream.h"
#include <string>

using namespace std;


namespace NLMISC {


/*
 * Constructor
 */
CBitMemStream::CBitMemStream( bool inputStream, uint32 defaultcapacity ) :
	CMemStream( inputStream, defaultcapacity ),
	_FreeBits( 8 )
{
}


/*
 * Copy constructor
 */
CBitMemStream::CBitMemStream( const CBitMemStream& other ) :
	CMemStream( other ),
	_FreeBits( other._FreeBits )
{
}


/*
 * Serialize a buffer
 */
void CBitMemStream::serialBuffer( uint8 *buf, uint len )
{
	uint i;
	uint32 v;
	if ( isReading() )
	{
		for ( i=0; i!=len; ++i )
		{
			v = 0;
			serial( v, 8 );
			buf[i] = (uint8)v;
		}
	}
	else
	{
		for ( i=0; i!=len; ++i )
		{
			v = (uint32)buf[i];
			serial( v, 8 );
		}
	}
}


/*
 * Transforms the message from input to output or from output to input
 */
void CBitMemStream::invert()
{
	CMemStream::invert();
	_FreeBits = 8;
}


/*
 * Serialize one bit
 */
void	CBitMemStream::serialBit( bool& bit )
{
	uint32 ubit=0;
	if ( isReading() )
	{
		serial( ubit, 1 );
		bit = ( ubit!=0 );
	}
	else
	{
		ubit = bit;
		serial( ubit, 1 );
	}
}


#define displayByteBits if ( false ) 
/*void displayByteBits( uint8 b, uint nbits, sint beginpos )
{
	string s1, s2;
	sint i;
	for ( i=nbits-1; i!=-1; --i )
	{
		s1 += ( (b >> i) & 1 ) ? "1" : "0";
	}
	nlinfo( "%s", s1.c_str() );
	for ( i=nbits; i>beginpos+1; --i )
	{
		s2 += " ";
	}
	s2 += "^";
	nlinfo( "%s beginpos=%u", s2.c_str(), beginpos );
}*/


#define displayDwordBits if ( false ) 
/*void displayDwordBits( uint32 b, uint nbits, sint beginpos )
{
	string s1, s2;
	sint i;
	for ( i=nbits-1; i!=-1; --i )
	{
		s1 += ( (b >> i) & 1 ) ? "1" : "0";
	}
	nlinfo( "%s", s1.c_str() );
	for ( i=nbits; i>beginpos+1; --i )
	{
		s2 += " ";
	}
	s2 += "^";
	nlinfo( "%s beginpos=%u", s2.c_str(), beginpos );
}*/


/*
 * Serialize only the nbits lower bits of value (when reading, please initialize to zero your variables)
 */
void	CBitMemStream::serial( uint32& value, uint nbits )
{
	nlassert( (nbits <= 32) && (nbits != 0) );

	if ( isReading() )
	{
		// Check that we don't read more than there is to read
		if ( lengthS()+(nbits+8-_FreeBits)/8 > lengthR())
		{
			throw EStreamOverflow();
		}
	
		// Clear high-order bits after _FreeBits
		uint8 v = *_BufPos; // & ((1 << _FreeBits) - 1);

		if ( nbits > _FreeBits )
		{
			nldebug( "Reading byte %u from %u free bits (%u remaining bits)", lengthS(), _FreeBits, nbits );
			value |= (v << (nbits-_FreeBits));
			++_BufPos;
			uint readbits = _FreeBits;
			displayByteBits( *_BufPos, 8, readbits-1 );
			_FreeBits = 8;
			serial( value, nbits - readbits );
		}
		else
		{
			nldebug( "Reading last byte %u from %u free bits (%u remaining bits)", lengthS(), _FreeBits, nbits );
			value |= (v >> (_FreeBits-nbits));
			if ( _FreeBits == nbits )
			{
				_FreeBits = 8;
				++_BufPos;
			}
			else
			{
				_FreeBits -= nbits;
			}
		}
	}
	else
	{
		// Resize if necessary
		if ( _FreeBits == 8 )
		{
			_Buffer.push_back();
			_BufPos = _Buffer.end() - 1;
			*_BufPos = 0;
		}

		// Clear high-order bits after nbits
		displayDwordBits( value, 32, nbits-1 );

		//uint32 mask = (-1 >> (32-nbits)); // does not work
		uint32 v;
		if ( nbits != 32 ) // arg of shl/sal/shr/sal ranges from 0 to 31
		{
			uint32 mask = (1 << nbits) - 1;
			v = value & mask;
		}
		else
		{
			v = value;
		}

		// Set
		if ( nbits > _FreeBits )
		{
			// Longer than the room in the current byte
			nldebug( "Writing byte %u into %u free bits (%u remaining bits)", lengthS(), _FreeBits, nbits );
			displayDwordBits( value, 32, nbits-1 );
			*_BufPos |= (v >> (nbits - _FreeBits));
			uint filledbits = _FreeBits;
			displayByteBits( *_BufPos, 8, filledbits-1 );
			_FreeBits = 8;
			serial( v, nbits - filledbits );
		}
		else
		{
			// Shorter or equal
			nldebug( "Writing last byte %u into %u free bits (%u remaining bits)", lengthS(), _FreeBits, nbits );
			displayByteBits( *_BufPos, 8, 7 );
			*_BufPos |= (v << (_FreeBits-nbits));
			displayByteBits( *_BufPos, 8, _FreeBits-1 );
			_FreeBits = ((_FreeBits-1 - nbits) % 8) + 1; // ((uint)-1) % 8 equals 7
		}

	}

}


/*
 * Serial float
 */
void	CBitMemStream::serial(float &b)
{
	uint32 uf=0;
	if ( isReading() )
	{
		serial( uf );
		b = *(float*)&uf;
	}
	else
	{
		uf = *(uint32*)&b;
		serial( uf );
	}
}


/*
 * Serial string
 */
void	CBitMemStream::serial(std::string &b) 
{
	uint32 length=0;

	// Serialize length
	if ( isReading() )
	{
		serial( length );
		b.resize( length );
	}
	else
	{
		length = b.size();
		serial( length );
	}

	// Serialize buffer
	if ( length != 0 )
	{
		serialBuffer( (uint8*)(&*b.begin()), length );
	}
}


/*
 * Specialisation of serialCont() for vector<bool>
 */
void CBitMemStream::serialCont(std::vector<bool> &cont)
{
	sint32	len=0;
	if(isReading())
	{
		serial(len);
		// special version for vector: adjut good size.
		contReset(cont);
		cont.reserve(len);

		for(sint i=0;i<len;i++)
		{
			bool	v;
			serialBit(v);
			cont.insert(cont.end(), v);
		}
	}
	else
	{
		len= cont.size();
		serial(len);

		std::vector<bool>::iterator it= cont.begin();
		for(sint i=0;i<len;i++, it++)
		{
			bool b = *it;
			serialBit( b );
		}
	}
}



} // NLMISC
